CXX=g++
SOURCES=$(wildcard src/*.cc)
HEADERS=$(wildcard include/*.h)
OBJECTS=$(SOURCES:%.cc=%.o)

LIB2_CFLAGS:=$(shell pkg-config --cflags libgit2)
LIB2_LIBS:=$(shell pkg-config --libs libgit2)

OUT=notescc

CXXFLAGS=-std=c++11 -O0 -g3 -Wall -Iinclude/ $(LIB2_CFLAGS) -Wall -Wextra -I/usr/include/rhash/

all: $(OUT)

# TODO: fix this later.
$(OBJECTS): $(HEADERS)

$(OUT): $(OBJECTS)
	$(CXX) -o $@ $^ -lrhash -lmarkdown $(LIB2_LIBS)

clean:
	rm -f $(OBJECTS) $(OUT)

indent:
	uncrustify --replace -c data/uncrustify.cfg $(SOURCES) $(HEADERS)
