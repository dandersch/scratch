#!/bin/bash

mkdir -p inc

#
# FIRST OPTION: #include a version of the binary file in text format
#
# convert all binary files into text files that can be #include'd as an unsigned char buffer
for i in $(ls "res")
do
    xxd -i "res/${i}" | sed -e 1d -e '$d' | sed -e '$d' > "inc/${i}"
done

#
# SECOND OPTION: link against a .o version of the binary file with predefined symbols
#
# convert all binary files into .o files that can be linked against
OBJECT_FILES=()
for i in $(ls "res")
do
    cd "res"
    OBJECT_FILES+="${i%.*}.o "
    ld -r -b binary "${i}" -o "../${i%.*}.o"
    # OR
    # objcopy --input binary --output elf64-x86-64 myfile.txt myfile.o
    cd ".."
done

clang -I inc -o main main.c ${OBJECT_FILES[*]} -lm
