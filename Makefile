CXX=g++
SOURCES=$(wildcard src/*.cc)
HEADERS=$(wildcard include/*.h)
OBJECTS=$(SOURCES:%.cc=%.o)
OUT=notescc
CXXFLAGS=-std=c++11 -O2 -g3 -Wall -Iinclude/

all: $(OUT)

# TODO: fix this later.
$(OBJECTS): $(HEADERS)

$(OUT): $(OBJECTS)
	$(CXX) -o $@ $^ -lrhash

clean:
	rm -f $(OBJECTS) $(OUT)

indent:
	uncrustify --replace -c data/uncrustify.cfg $(SOURCES) $(HEADERS)
