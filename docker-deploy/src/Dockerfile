FROM ubuntu
RUN apt-get update 
RUN apt-get install -y g++ make
RUN mkdir -p /var/log/erss
ADD . /code/
WORKDIR /code
RUN ["chmod", "+x", "run.sh"] 
ENTRYPOINT ["./run.sh"]