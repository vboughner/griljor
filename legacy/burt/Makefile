# Makefile for burt, an eliza-like response program by Van A. Boughner
#

CC = cc
CFLAGS = -g


burt:	main.o io.o load.o person.o response.o

testburt: testburt.o main.o io.o load.o person.o response.o
	${CC} ${CFLAGS} testburt.o main.o io.o load.o person.o response.o -o testburt

tar:	*.c *.h *.text Makefile
	tar cvf burt.tar *.c *.h *.text Makefile

tags:	*.c *.h
	ctags *.c *.h

testburt.o main.o io.o load.o person.o response.o: burt.h extern.h
