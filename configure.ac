AC_INIT([notescc], [0.14.7], [qball@gmpclient.org])

AC_CONFIG_SRCDIR([src/main.cc])

##
# Setup automake to be silent and in foreign mode.
##
AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects dist-xz])
AM_SILENT_RULES([yes])

##
# Check for compiler
##
m4_include([m4/ax_cxx_compile_stdcxx_11.m4])
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11 
AM_PROG_CC_C_O

AC_USE_SYSTEM_EXTENSIONS
##
# Readline
##

AC_CHECK_LIB([readline], [readline])
AC_CHECK_HEADERS(readline/history.h readline/readline.h)

AC_CHECK_LIB([markdown], [markdown])
AC_CHECK_HEADERS(mkdio.h)
##
# Check dependencies
##
PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([libgit2], [libgit2])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
