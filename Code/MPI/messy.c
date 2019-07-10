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
  if( myRank == 0 ) {
    sum = myNumber;
    int received = -1;
    for( int i=1; i<comSize; i++ ) {
      MPI_Recv( &received, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
      sum += received;
    }
    avg = sum / comSize;
  
    MPI_Bcast( &avg, 1, MPI_INT, 0, MPI_COMM_WORLD );
  } else {
    MPI_Send( &myNumber, 1, MPI_INT, 0, 0, MPI_COMM_WORLD );
    MPI_Bcast( &avg, 1, MPI_INT, 0, MPI_COMM_WORLD );	
  }
  
  printf( "RANK-%d - Average: %d.\n", myRank, avg );

  //They compute the difference between their number and
  // the average
  int diff = myNumber - avg;

  printf( "RANK-%d - Diff is: %d.\n", myRank, diff );

  //Rank 0, looks for the largest difference and print it
  if( myRank == 0 ) {
    int *diffs = (int*) malloc( sizeof(int) * comSize );
    diffs[0] = diff;
    for( int i=1; i<comSize; i++ ) {
      MPI_Recv( &diffs[i], 1, MPI_INT, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    }
    int max = -1;
    for( int i=0; i<comSize; i++ ) {
      if( diffs[i] > max ) max = diffs[i] ;
    }
    printf( "RANK-%d - Max diff is: %d.\n", myRank, max );
    free(diffs);
  } else {
    MPI_Send( &diff, 1, MPI_INT, 0, 0, MPI_COMM_WORLD );
  }

  MPI_Finalize();

}

    

  

  

  
