#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {

  // Will contain the size of the communicator and the process rank
  int comSize, myRank = -1;

  //Initialize and "start" MPI
  MPI_Init(NULL,NULL);

  // Get the communicator size and process rank
  MPI_Comm_size(MPI_COMM_WORLD, &comSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

  // Eponymous message
  printf("Hello world from rank %d"
	 " out of %d processors\n",
         myRank, comSize);
	
  // Finish current exchanges and "stop" MPI
  MPI_Finalize();
}
