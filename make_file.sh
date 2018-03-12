#! /bin/bash

clang++ -std=c++1z -Wc++11-extensions -o x86-to-6502 src/main.cpp && \
clang++ -std=c++1z -c -O3 -o- -Wall -Wextra -m32 -march=i386 -ggdb -S $1 > $1.x86.asm && \
cat $1.x86.asm | ./x86-to-6502 > $1.6502.asm && \
cat $1.6502.asm | sed -e "/^\t\..*$/d" > $1.asm && \
cat $1.asm

