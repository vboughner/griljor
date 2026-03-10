# Makefile for mask making program

CC = gcc
CFLAGS = -O

mkmask:	main.o mask.o findbits.o X11ReadBitmapFile.o X11WriteBitmapFile.o
	${CC} main.o mask.o X11ReadBitmapFile.o X11WriteBitmapFile.o \
	findbits.o -o mkmask -lm
