# MACROS
CXX = g++
CXXFLAGS = -O3 -Wall -std=c++11
LIBS = -pthread
OBJS = server.o client.o
EXES = server client

# TARGETS
all: $(EXES)

server: server.o
	$(CXX) -o server server.o $(CXXFLAGS) $(LIBS)

server.o: server.cc 
	$(CXX) -o server.o -c server.cc $(CXXFLAGS) $(LIBS)

client: client.o
	$(CXX) -o client client.o $(CXXFLAGS) $(LIBS)

client.o: client.cc
	$(CXX) -o client.o -c client.cc $(CXXFLAGS) $(LIBS)

# REMOVAL
clean:
	rm -rf $(OBJS) $(EXES)