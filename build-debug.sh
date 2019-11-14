#!/bin/sh

make clean
make distclean

./configure --prefix=/dbg CPPFLAGS=-DDEBUG CXXFLAGS="-g -O0" CFLAGS="-g -O0" && make