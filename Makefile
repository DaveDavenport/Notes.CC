CXX=g++
SOURCES=$(wildcard src/*.cc)
HEADERS=$(wildcard include/*.h)
OBJECTS=$(SOURCES:%.cc=%.o)

PREFIX=~/.local/

##
# Dependencies
##

# Libgit2
LIB2_CFLAGS:=$(shell pkg-config --cflags libgit2)
LIB2_LIBS:=$(shell pkg-config --libs libgit2)

READLINE_CFLAGS:=
READLINE_LIBS:= -lreadline

MARKDOWN_CFLAGS:=
MARKDOWN_LIBS:=-lmarkdown

OUT=notescc

CXXFLAGS=-std=c++11 -O0 -g3 -Wall -Wextra -Wno-missing-field-initializers\
         -Iinclude/\
         $(LIB2_CFLAGS)\
         $(READLINE_CFLAGS)\
         $(MARKDOWN_CFLAGS)

LDLIBS=$(LIB2_LIBS) $(READLINE_LIBS) $(MARKDOWN_LIBS)

all: $(OUT)

# TODO: fix this later.
$(OBJECTS): $(HEADERS)

$(OUT): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDLIBS)

clean:
	rm -f $(OBJECTS) $(OUT)

indent:
	uncrustify --replace -c data/uncrustify.cfg $(SOURCES) $(HEADERS)

$(PREFIX)/bin/$(OUT): $(OUT)
	cp $^ $@

install: ${PREFIX}/bin/$(OUT)
