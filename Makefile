all:	x86-to-6502

x86-to-6502:	Makefile src/main.cpp
	clang++ -std=c++1z -Wc++11-extensions -o x86-to-6502 src/main.cpp

# Convenient transformation rules for compiling C files into x86, 6502 and 45GS02
# assembly language
%.x86:	%.c
	clang++ -std=c++1z -c -O3 -o $@ -Wall -Wextra -m32 -march=i386 -ggdb -S $<

%.6502:	%.x86
	./x86-to-6502 -m 6502 < $< > $@

%.45gs02:	%.x86
	./x86-to-6502 -m 45gs02 < $< > $@
