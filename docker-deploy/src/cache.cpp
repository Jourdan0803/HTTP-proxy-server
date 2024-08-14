#include "cache.hpp"

// cache 需要使用 mutex
// 1. 检查在不在cache
//  1.1 在cache
//      判断 revalidate
//      1.1.1 must revalidate
//          1.1.1.1 检查是否过期 （将etag，last modify加入request， 发给server）
//              1.1.1.1.1 如果200， 需要修改+update cache （之前的需要erase？还是直接update）
//              1.1.1.1.2 如果304，已经是最新版本 -> 使用max-age （或者 expiration？）检查是否过期
//                        检查方法： 得到data的时间 + max-age 和curr进行对比
//      1.1.2 不需要 revalidate
//          1.1.2.1 检查max-age 和 expiration date
//  1.2 不在cache ->  判断可不可store
//      1.2.1 若可以，加入cache
//      1.2.2 若不可以，写入 log: not cacheable
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

void Cache::addResponse(Request req, Response res, int request_id) {
    if (res.privateCache) {
        log << request_id << ": not cacheable because the response is private" << endl;
        return;
    }
    if (res.noStore) {
        log << request_id << ": not cacheable because the response is no-store" << endl;
        return;
    }
    // if (res.noCache) {
    //     log << request_id << ": not cacheable because the response is no-cache" << endl;
    // }
    if (res.eTag.empty() || res.lastModified.empty()) {
        // cout << "etag: " << res.eTag << endl;
        log << request_id << ": not cacheable because there is no eTag or last modified date." << endl;
        return;
    }
    // cout<<"in add"<<endl;
    lock_guard<mutex> guard(cacheMutex);
    // cout<<"in add"<<endl;
    // first in first out if full
    if (cacheResponse.find(req.getURL()) == cacheResponse.end()) {
        if (fifoCache.size() >= 100) {
            log << request_id <<": NOTE evicted " << fifoCache.front() << " from cache" << endl;
            cacheResponse.erase(fifoCache.front());
            fifoCache.pop_front();
        }
        // add to cache
        cacheResponse[req.getURL()] = res;
        fifoCache.push_back(req.getURL());
        if (res.mustRevalidate||res.maxAge == 0 ||res.noCache) {
            log << request_id << ": cached, but requires re-validation" << endl;
        }
        else {
            log << request_id << ": cached, expires at " << res.expirationTime  << endl;
        }
    }
    else {
        // update cache
        log << request_id <<": NOTE evicted " << req.getURL() << " from cache" << endl;
        cacheResponse[req.getURL()] = res;
        fifoCache.remove(req.getURL());
        fifoCache.push_back(req.getURL());
        if (res.mustRevalidate||res.maxAge == 0 ||res.noCache) {
            log << request_id << ": cached, but requires re-validation" << endl;
        }
        else {
            log << request_id << ": cached, expires at " << res.expirationTime  << endl;
        }
    }
}

Response * Cache::getResponse(Request req, char * requestmsg, int len, int fd, int request_id) {
    // lock_guard<mutex> guard(cacheMutex);
    Response * res = &cacheResponse[req.getURL()];

    // cout << "res->expirationTime: " << res->expirationTime << endl;
    bool expired = isExpired(res->expirationTime,
                                res->receivedTime,
                                res->maxAge);
    // cout << "expired: " << expired << endl;
    if (res->mustRevalidate || expired || res->maxAge == 0 ||res->noCache) {
        // check if the lastest version
        if (expired) {
            log << request_id << ": in cache, but expired at " << res->expirationTime << endl;
        }
        else if (res->mustRevalidate|| res->maxAge == 0 ||res->noCache) {
            log << request_id << ": in cache, requires re-validation";
            if (res->maxAge == 0) {
                log << " because max-age is 0";
            }
            else if (res->noCache) {
                log << " because this response is no cache";
            }
            log << endl;
        }
        // send req
        string sendReq = req.requestContent;
        if (!res->eTag.empty()) {
            sendReq += "If-None-Match: " + res->eTag + "\r\n";
        }
        if (!res->lastModified.empty()) {
            sendReq += "If-Modified-Since: " + res->lastModified + "\r\n";
        }
        //  cout<< "新的request"<<sendReq << endl;
        //  cout<<"之前的"<<requestmsg<<endl;
        log << request_id << ": Requesting " << req.getRequestLine() << "\" from " << req.getHost() << endl;
        if (send(fd, sendReq.c_str(), sendReq.length(), 0) < 0) {
        // if (send(fd, requestmsg, len, 0) <= 0) {
            cerr << "ERROR sending request." << endl;
            exit(EXIT_FAILURE);
        }

        // receive response
        char servermsg[65536] = {0};
        int lenServer = recv(fd, servermsg, sizeof(servermsg), 0);
        // cout<<servermsg<<endl;
        Response * recvRes = new Response(servermsg);
        string status_code = recvRes->status.substr(0, 3);
        log << request_id << ": Received \"" << recvRes->line << "\" from " << req.getHost() << endl;

        // cout << "status_code: "<< status_code<< endl;

        if (status_code == "200" || expired) {
            addResponse(req, *recvRes, request_id);
            //log << request_id << ": Responding \"" << recvRes->line << "\" from " << req.getHost() << endl;
            return recvRes;
        }
        if (status_code == "304") {
            // already lastest version, and not expired
            log << request_id << ": in cache, valid" << endl;
            return res;
        }
    }

    log << request_id << ": in cache, valid" << endl;
    return res;
}

bool Cache::isExpired(const std::string expireTime, 
                      const std::string receiveTime,
                      const size_t maxAge) {

    // cout << "expireTime: " << expireTime << endl;
    // cout << "receiveTime: " << receiveTime << endl;
    // cout << "maxAge: " << maxAge << endl;

    time_t curr = time(nullptr);

    std::tm exp_tm = {};
    memset(&exp_tm, 0, sizeof(exp_tm));
    strptime(expireTime.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &exp_tm);
    // strptime(expireTime.c_str(), "%a %b %d %H:%M:%S %Y", &exp_tm);
    exp_tm.tm_isdst = 0;
    time_t expTime = timegm(&exp_tm);

    std::tm rec_tm = {};
    memset(&rec_tm, 0, sizeof(rec_tm));
    // strptime(receiveTime.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &rec_tm);
    strptime(receiveTime.c_str(), "%a %b %d %H:%M:%S %Y", &rec_tm);
    rec_tm.tm_isdst = 0;
    time_t recTime = timegm(&rec_tm);
    time_t maxAgeExpireTime = recTime + maxAge;

    // cout << "curr: " << curr << endl;
    // cout << "expTime: " << expTime << endl;
    // cout << "recTime: " << recTime << endl;
    // cout << "maxAgeExpireTime: " << maxAgeExpireTime << endl;


    return difftime(curr, expTime) > 0 || difftime(curr, maxAgeExpireTime) > 0;
}