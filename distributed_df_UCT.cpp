#include <iostream>
#include <mpi.h>
using namespace std;

#include "connect_four.cpp"

int main(int argc, char** argv){
	int rank, size;

	/* Initailize MPI environemnt */
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	/* Define the varibales we need in advance */
	double start_time, end_time;
	MPI_Request request;
	MPI_Status status;


	MPI_Barrier(MPI_COMM_WORLD);
}


