
CC = rgcc
CFLAGS = -g
NAME = obvert
OBJS = obvert.o objstore.o vsys.o vline.o objects.o lib.o objheader.o \
	X11ReadBitmapFile.o error.o
SRCS = obvert.c objstore.c vsys.c vline.c objects.c lib.c objheader.o \
	X11ReadBitmapFile.c error.c
LIBS = -lm

${NAME}: ${OBJS}
	${CC} ${CFLAGS} ${OBJS} ${LIBS} -o ${NAME}


obvert.o: ObjDiag.t dialog.h objects.h
objstore.o: ObjDiag.t dialog.h objects.h
objects.o: objects.h
