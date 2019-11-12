#!/bin/sh
rm -rf release
mkdir release
cd release && ../configure CPPFLAGS=-DNDEBUG && make 