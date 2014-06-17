CXX=clang++
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:%.cc=%.o)
OUT=notescc

$(OUT): $(OBJECTS)
	$(CXX) -o $@ $^
