OBJS = main.o fp.o
CC = g++
DEBUG = -g -O3 -ansi -pedantic
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

fpexec : $(OBJS)
	$(CC) $(LFLAGS) $(OBJS) -o fpexec

main.o : main.cpp fp.cpp fp.h
	$(CC) $(CFLAGS) main.cpp

fp.o : fp.cpp fp.h
	$(CC) $(CFLAGS) fp.cpp

clean:	
	\rm *.o fpexec
