CXX=g++
SOURCES=$(wildcard src/*.cc)
HEADERS=$(wildcard include/*.h)
OBJECTS=$(SOURCES:%.cc=%.o)
OUT=notescc
CXXFLAGS=-std=c++11 -g3 -Wall -Iinclude/

$(OUT): $(OBJECTS)
	$(CXX) -o $@ $^

clean:
	rm -f $(OBJECTS) $(OUT)
