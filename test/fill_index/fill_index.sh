#!/bin/sh

g++ -m32 -g -c fill_index.cc
g++ -m32 -g -o fill_index fill_index.o ../../util/coding.o ../../util/crc32c.o ../../util/clock.o
