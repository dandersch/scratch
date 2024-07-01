#!/bin/bash


clang -E -std=c99 -Wall serialize.c -o pp.h
clang -std=c99 -Wall serialize.c -o test && ./test
