#!/bin/sh
rm -rf debug
mkdir debug
make distclean
cd debug && ../configure --prefix=/dbg CPPFLAGS=-DDEBUG CXXFLAGS="-g -O0" CFLAGS="-g -O0" && make