CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -fPIC
LDFLAGS = -ldl

SERVER_TARGET = server
SERVER_OBJS = main.o common.o server.o module.o

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CXX) -o $@ $(SERVER_OBJS) $(LDFLAGS)

clean:
	rm -f $(SERVER_TARGET) $(SERVER_OBJS) $(MODULES) test_server

.PHONY: clean
