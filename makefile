# Makefile for a4, CS480
# Mason Leonard 818030805

PROGRAM = rideshare
CC = gcc
CFLAGS = -g -std=c11


${PROGRAM}:      producer.o consumer.o io.o main.o
	${CC} -pthread -lrt -o rideshare producer.o consumer.o io.o main.o

producer.o: 	ridesharing.h

consumer.o:	ridesharing.h

io.o:	io.h 

main.o:	ridesharing.h

splint:
	splint -warnposix +trytorecover -weak ${PROGRAM}.c

clean:
	rm -f *.o ${PROGRAM}

