CXX=g++
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:%.cc=%.o)
OUT=notescc
CXXFLAGS=-std=c++11 -g3 -Wall

$(OUT): $(OBJECTS)
	$(CXX) -o $@ $^
