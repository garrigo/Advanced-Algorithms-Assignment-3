CC = g++
CFLAGS = -Wall -g -pthread
 
# ****************************************************
# Targets needed to bring the executable up to date

all: main clean

main: main.o read-obj.o
	$(CC) $(CFLAGS) -o main main.o read-obj.o
 
# The main.o target can be written more simply
 
main.o: main.cpp read-obj.h
	$(CC) $(CFLAGS) -c main.cpp
 
read-obj.o: read-obj.h

clean:
	rm *.o