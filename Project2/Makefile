# MACROS
CXX = g++
CXXFLAGS = -O3 -Wall -std=c++11
LIBS = -pthread
OBJS = sender.o receiver.o test.o
EXES = sender receiver test

# TARGETS
all: $(EXES)

sender: sender.o
	$(CXX) -o sender sender.o $(CXXFLAGS) $(LIBS)

sender.o: sender.cc util.h
	$(CXX) -o sender.o -c sender.cc $(CXXFLAGS) $(LIBS)

receiver: receiver.o
	$(CXX) -o receiver receiver.o $(CXXFLAGS) $(LIBS)

receiver.o: receiver.cc util.h
	$(CXX) -o receiver.o -c receiver.cc $(CXXFLAGS) $(LIBS)

test: test.o
	$(CXX) -o test test.o $(CXXFLAGS) $(LIBS)

test.o: test.cc util.h
	$(CXX) -o test.o -c test.cc $(CXXFLAGS) $(LIBS)

# REMOVAL
clean:
	rm -rf $(OBJS) $(EXES)