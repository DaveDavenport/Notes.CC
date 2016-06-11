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
AC_PROG_CXX([clang++ g++ c++])
AX_CXX_COMPILE_STDCXX_11 
AM_PROG_CC_C_O

AC_USE_SYSTEM_EXTENSIONS
dnl ---------------------------------------------------------------------
dnl Base CFLAGS
dnl ---------------------------------------------------------------------
AM_CXXFLAGS="-Wall -Wextra -Wparentheses -Winline -pedantic  -Wunreachable-code"
##
# Readline
##

AC_CHECK_LIB([readline], [readline],,
        AC_MSG_ERROR("Could not find readline library"))
AC_CHECK_HEADERS(readline/history.h readline/readline.h, ,
        AC_MSG_ERROR("Could not find readline header files"))

AC_CHECK_LIB([markdown], [markdown], , AC_MSG_ERROR("Could not find markdown library"))
AC_CHECK_HEADERS(mkdio.h,,AC_MSG_ERROR("Could not find libmarkdown header file: mkdio.h"))
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
