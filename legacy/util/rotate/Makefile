# Bitmap Rotation routines
# Largely done by Albert C. Baker III
# February 1990

CC = gcc

rotate: X11ReadBitmapFile.o X11WriteBitmapFile.o dobits.o hflip.o \
	main.o rotate.o topology.o vflip.o
	${CC} -g X11ReadBitmapFile.o X11WriteBitmapFile.o dobits.o hflip.o \
	main.o rotate.o topology.o vflip.o -o rotate -lm
