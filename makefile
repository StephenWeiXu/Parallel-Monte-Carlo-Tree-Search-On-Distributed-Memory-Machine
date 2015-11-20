CC = g++
CFLAGS = -g -std=c++11

all:
	$(CC) connect_four.cpp mcts.cpp $(CFLAGS) -o connect_four
