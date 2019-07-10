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

  int outFriend = (myRank+1) % comSize;
  int inFriend = (myRank + comSize - 1) % comSize;
  MPI_Request req;
  
  MPI_Isend(  &secret, 1, MPI_INT, outFriend, 0, MPI_COMM_WORLD, &req );
  MPI_Recv(  &friendSecret, 1, MPI_INT, inFriend, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE );

  printf( "My rank is %d and I received the secret message: %d.\n", myRank, friendSecret);
  
  MPI_Wait( &req, MPI_STATUS_IGNORE );
  
  MPI_Finalize();
}
