CXX=g++
SOURCES=$(wildcard src/*.cc)
HEADERS=$(wildcard include/*.h)
OBJECTS=$(SOURCES:%.cc=%.o)
OUT=notescc
CXXFLAGS=-std=c++11 -O0 -g3 -Wall -Iinclude/ -I/usr/include/ -Wall -Wextra -I/home/mkoedam/.local/include/ -I/usr/include/rhash/

all: $(OUT)

# TODO: fix this later.
$(OBJECTS): $(HEADERS)

$(OUT): $(OBJECTS)
	$(CXX) -o $@ $^ -lrhash -lmarkdown -L/home/mkoedam/.local/lib/ -lgit2

clean:
	rm -f $(OBJECTS) $(OUT)

indent:
	uncrustify --replace -c data/uncrustify.cfg $(SOURCES) $(HEADERS)
