#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include <time.h>

int main(int argc, char** argv) {

  int comSize, myRank = -1;

  MPI_Init(NULL,NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comSize);
  MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

  srand( time(NULL) + myRank );
  int secret = rand();
  int friendSecret = -1;

  printf( "My rank is %d and my secret is: %d.\n", myRank, secret );


  if( myRank == 0 ) {
    MPI_Send( &secret, 1, MPI_INT, 1, 0, MPI_COMM_WORLD );
    MPI_Recv( &friendSecret, 1, MPI_INT, comSize-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
  } else if( myRank != comSize - 1 ) {
    MPI_Recv( &friendSecret, 1, MPI_INT, myRank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    MPI_Send( &secret, 1, MPI_INT, myRank + 1, 0, MPI_COMM_WORLD );
  } else {
    MPI_Recv( &friendSecret, 1, MPI_INT, myRank-1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    MPI_Send( &secret, 1, MPI_INT, 0, 0, MPI_COMM_WORLD );	
  }

  printf( "My rank is %d and I received the secret message: %d.\n", myRank, friendSecret);

  MPI_Finalize();
}
