LIBS=-lbfd
CXXFLAGS=-std=c++14
CXX=g++
OBJ=example
BIN_DIR=./bin
OBJ_DIR=./bin

.PHONY: all clean

all: $(OBJ)

elfreader.o: ./src/*.cpp
	$(CXX) $(CXXFLAGS) -c -o $(BIN_DIR)/$@ $^ $(LIBS)

example: elfreader.o example.cpp
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/$@ $(OBJ_DIR)/$^ $(LIBS)


clean:
	rm -f ./bin/*