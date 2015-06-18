##
# Rofi the program
##
bin_PROGRAMS=notescc

LIBS=\
    -lreadline\
    -lmarkdown

AM_CXXFLAGS=\
    -I$(top_srcdir)/include/\
    -I$(top_builddir)/\
	-Wno-write-strings -fpermissive

notescc_SOURCES=\
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

##
# Indent
##

indent: ${notescc_SOURCES}
	uncrustify -c ${top_srcdir}/data/uncrustify.cfg --replace $^


##
# Extra files to pack in distribution
##
EXTRA_DIST=notesccrc
EXTRA_DIST+=css/markdown.css
