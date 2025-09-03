CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -fPIC
LDFLAGS = -ldl

SERVER_TARGET = server
SERVER_OBJS = main.o common.o server.o module.o
MODULES = diskfree.so issue.so

all: $(SERVER_TARGET) $(MODULES)

$(SERVER_TARGET): $(SERVER_OBJS)
	$(CXX) -o $@ $(SERVER_OBJS) $(LDFLAGS)

clean:
	rm -f $(SERVER_TARGET) $(SERVER_OBJS) $(MODULES) test_server

# modules
diskfree.so: diskfree.cpp
	$(CXX) $(CXXFLAGS) -shared -o $@ diskfree.cpp

issue.so: issue.cpp
	$(CXX) $(CXXFLAGS) -shared -o $@ issue.cpp

.PHONY: clean
