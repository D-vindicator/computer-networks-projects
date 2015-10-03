# MACROS
CXX = g++
CXXFLAGS = -O3 -Wall -std=c++11
LIBS = -pthread
OBJS = Server.o Client.o
EXES = Server Client

# TARGETS
all: $(EXES)

Server: Server.o
	$(CXX) -o Server Server.o $(CXXFLAGS) $(LIBS)

Server.o: Server.cc util.h
	$(CXX) -o Server.o -c Server.cc $(CXXFLAGS) $(LIBS)

Client: Client.o
	$(CXX) -o Client Client.o $(CXXFLAGS) $(LIBS)

Client.o: Client.cc util.h
	$(CXX) -o Client.o -c Client.cc $(CXXFLAGS) $(LIBS)

# REMOVAL
clean:
	rm -rf $(OBJS) $(EXES)