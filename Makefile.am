##
# Rofi the program
##
bin_PROGRAMS=notescc

LIBS=\
    @libgit2_LIBS@\
    -lreadline

AM_CXXFLAGS=\
    @libgit2_CFLAGS@\
    -I$(top_srcdir)/include/\
    -I$(top_builddir)/\
	-I$(top_srcdir)/hoedown/src/


HOEDOWN_SOURCES=\
	hoedown/src/autolink.c\
	hoedown/src/buffer.c\
	hoedown/src/document.c\
	hoedown/src/escape.c\
	hoedown/src/html.c\
	hoedown/src/html_blocks.c\
	hoedown/src/html_smartypants.c\
	hoedown/src/stack.c\
	hoedown/src/version.c\
	hoedown/src/autolink.h\
	hoedown/src/buffer.h\
	hoedown/src/document.h\
	hoedown/src/escape.h\
	hoedown/src/html.h\
	hoedown/src/stack.h\
	hoedown/src/version.h

main_SOURCES=\
    src/main.cc\
    src/Note.cc\
    src/Project.cc\
    src/Colors.cc\
    src/Filter.cc\
    src/Settings.cc\
    src/IDStorage.cc\
    src/TableView.cc\
    include/Note.h\
    include/Colors.h\
    include/Project.h\
    include/Settings.h\
    include/TableView.h\
    include/IDStorage.h\
    include/Filter.h

notescc_SOURCES=\
	$(main_SOURCES)\
	$(HOEDOWN_SOURCES)

##
# Indent
##

indent: ${main_SOURCES}
	uncrustify -c ${top_srcdir}/data/uncrustify.cfg --replace $^


##
# Extra files to pack in distribution
##
EXTRA_DIST=notesccrc
