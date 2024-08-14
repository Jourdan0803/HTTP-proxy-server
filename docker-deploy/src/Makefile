CC = g++
CFLAGS = -g -pthread
TARGET = main
SRCS = main.cpp server.cpp request.cpp response.cpp proxy.cpp cache.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET) *~ *.o *.so client master testRequest testResponse

run: $(TARGET)
	./$(TARGET)
