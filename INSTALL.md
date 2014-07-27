# Installation Guide

## Dependencies

Building environment:

 * A C++ compiler that supports the c++11 standard. This can either be clang++ or an up to date gcc
   compiler.
 * autoconf
 * make
 * automake

Libraries:

 * libreadline
 * libmarkdown (libmarkdown2 on debian)
 * libgit2 (0.20 or later)


## Compilation (from git)

First create the build infrastructure

```
autoreconf -i
```

It is preferable (but not needed) to build in a build directory.

```
mkdir build
cd build
```

Run configure script:

```
../configure
```

Build `Notes.CC`

```
make
```

Install it:

```
make install
```

## Tweaking compilation

The following `hints` can be combined.

### Installing in home directory

```
../configure --prefix=${HOME}/.local/
```
This will place the binary in `~/.local/bin/`, make sure this is part of your `$PATH`

### Using clang++ instead of g++

```
CXX=clang++ ../configure
```

Configure should now check clang++ as a valid compiler.
