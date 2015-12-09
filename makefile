CC = g++
CFLAGS = -g -O -fdiagnostics-color=always -Wall -Werror -std=c++11

all:
	$(CC) main.cpp gomoku.cpp mcts.cpp  $(CFLAGS) -o main.o
clean:
	rm *.o
test:
	./main.o
