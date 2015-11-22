CC = g++
CFLAGS = -g -std=c++11

all:
	$(CC) gomoku.cpp mcts.cpp $(CFLAGS) -o gomoku
