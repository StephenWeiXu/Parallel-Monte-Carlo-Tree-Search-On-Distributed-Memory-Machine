CC = g++
CFLAGS = -g -O -Wall -Werror -std=c++11

all:
	$(CC) gomoku.cpp $(CFLAGS) -o gomoku.o
clean:
	rm *.o
test:
	./gomoku.o
