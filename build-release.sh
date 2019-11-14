#!/bin/sh

make clean
make distclean

./configure CPPFLAGS=-DNDEBUG && make 