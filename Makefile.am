##
# Rofi the program
##
bin_PROGRAMS=notescc

LIBS=\
    @libgit2_LIBS@\
    -lreadline\
    -lmarkdown

AM_CXXFLAGS=\
    @libgit2_CFLAGS@\
    -I$(top_srcdir)/include/\
    -I$(top_builddir)/

notescc_SOURCES=\
    src/notescc.cc\
    src/Note.cc\
    src/Project.cc\
    src/Colors.cc\
    src/Filter.cc\
    src/TableView.cc\
    src/settings.cc\
    include/Note.h\
    include/Colors.h\
    include/Project.h\
    include/settings.h\
    include/TableView.h\
    include/Filter.h

##
# Indent
##

indent: ${notescc_SOURCES}
	uncrustify -c ${top_srcdir}/data/uncrustify.cfg --replace $^
