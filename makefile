CC = g++
CFLAGS = -g -O -fdiagnostics-color=always -Wall -Werror -std=c++11

all:
	$(CC) mcts.cpp $(CFLAGS) -o mcts.o
	$(CC) gomoku.cpp $(CFLAGS) -o gomoku.o
	$(CC) mcts.o gomoku.o -o output.o
clean:
	rm *.o
test:
	./output.o
