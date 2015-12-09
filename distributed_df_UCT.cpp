#include <iostream>
#include <mpi.h>
#include <queue>
using namespace std;

#include "gomoku.h"

/*
The important functionalities this implementation will use:
	1. Zobrist hash function
	2. Transposition Table Driven Work Scheduling (TDS)
*/


enum job_t {SEARCH, REPORT};

struct Job_struct{
	job_t type;
	Node<GomokuState> job_node;
};


void zobrist_hash(Node<GomokuState> *node){

}

void look_up_hash_table(Node<GomokuState> *node){

}

void update_hash_table(){

}

void TDS_UCT_main(){

}



int main(int argc, char** argv){
	int rank, size;

	/* Initailize MPI environemnt */
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	/* Define the data structures we need */
	queue<Job_struct> Job_queue;


	/* Define the varibales we need in advance */
	double start_time, end_time;
	MPI_Request request;
	MPI_Status status;


	MPI_Barrier(MPI_COMM_WORLD);
	if (rank == 0){
		start_time = MPI_Wtime();

		/* root processor initialize */
		bool human_player = true;
		ComputeOptions player1_options, player2_options;
		player1_options.max_iterations = 100000;
		player1_options.verbose = true;
		player2_options.max_iterations =  10000;
		player2_options.verbose = true;

		GomokuState root_state;
		while (root_state.has_moves()) {
			cout << endl << "State: " << root_state << endl;

			GomokuState::Move move = {-1, -1};
			if (root_state.player_to_move == 1) {
				cout << "Computer is caculating its move..." << endl;
				auto moves = root_state.get_moves();
				
				mt19937_64::result_type initial_seed = 1012411 * 0 + 12515;
				mt19937_64 random_engine(initial_seed);
				auto root_node = new Node<State>(root_state);

				// generate child and send child to its home processor
			}
		}		

	}



}


