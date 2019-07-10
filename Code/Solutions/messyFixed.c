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

  //They all pick a number
  int myNumber = rand() % 100;

  printf( "RANK-%d - Number: %d.\n", myRank, myNumber );

  int sum = -1;
  int avg = -1;

  //They sent their number to '0'. It computes the average and
  //broadcast the result
  MPI_Reduce( &myNumber, &sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD );
  if( myRank == 0 ) {
    avg = sum / comSize;
  }
  MPI_Bcast( &avg, 1, MPI_INT, 0, MPI_COMM_WORLD );


  //They compute the difference between their number and
  // the average
  int diff = myNumber - avg;

  printf( "RANK-%d - Diff is: %d.\n", myRank, diff );

  //Rank 0, looks for the largest difference and print it
  int max = -1;
  MPI_Reduce( &diff, &max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD );
  if( myRank == 0 ) {
    printf( "RANK-%d - Max diff is: %d.\n", myRank, max );
  }

  MPI_Finalize();

}

    
