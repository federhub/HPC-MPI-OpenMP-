#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <mpi.h>

#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#define sqr( a ) ( (a) * (a) )


const int numpoints=1024;
const double eps=0.00001;


void write_to_bmp( int n, double **tmptab, int iter,
                   double minval, double maxval );


int main(int argc, char *argv[])
{
  int mpi_rank, mpi_size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  int i, j;

  int N = numpoints-1;
  double h = 1. / (double)N;

  // make sure domain is divisible in equal-sized domains
  assert( numpoints % mpi_size == 0 );
  int num_local_lines = numpoints / mpi_size;
  int num_allocated_lines = num_local_lines + 2;

  // allocation of raw memory
  double* u_memory  = (double*) malloc(num_allocated_lines*numpoints*sizeof(double));
  double* utmp_memory = (double*) malloc(num_allocated_lines*numpoints*sizeof(double));
  double* f_memory  = (double*) malloc(num_allocated_lines*numpoints*sizeof(double));
  double* global_matrix_memory = 0;
  if (mpi_rank==0) {
    global_matrix_memory = (double*) malloc(numpoints*numpoints*sizeof(double));
  }

  // allocation of arrays of pointers
  double** u  = (double**) malloc(num_allocated_lines*sizeof(double*));
  double** utmp = (double**) malloc(num_allocated_lines*sizeof(double*));
  double** f  = (double**) malloc(num_allocated_lines*sizeof(double*));

  // creation of matrix structure
  for(i=0; i<num_allocated_lines; i++) {
    u [i] = (double*) u_memory + i*numpoints;
    utmp[i] = (double*) utmp_memory + i*numpoints;
    f [i] = (double*) f_memory + i*numpoints;
  }

  double** global_matrix = 0;
  if (mpi_rank==0) {
    global_matrix = (double**) malloc(numpoints*sizeof(double*));
    for(i=0; i<numpoints; i++) {
      global_matrix[i] = (double*) global_matrix_memory + i*numpoints;
    }
  }

  // initialization of local matrices
  for(i=0; i<num_allocated_lines; i++) {
    for(j=0; j<numpoints; j++) {
      utmp[i][j] = 0;
      u[i][j] = 0;
      f[i][j] = 0;
    }
  }
  if (mpi_rank==0) {
    double pi = acos(-1.);
    for(i=0; i<numpoints; i++) {
     for(j=0; j<numpoints; j++) {
        global_matrix[i][j] = -2.*100. * sqr(pi) * sin(10.*pi*i*h) * sin(10.*pi*j*h);
      }
    }
  }
  MPI_Scatter( global_matrix_memory, num_local_lines*numpoints, MPI_DOUBLE,
               f[1], num_local_lines*numpoints, MPI_DOUBLE, 0, MPI_COMM_WORLD );

  int k=0;
  double l2 = 1.;
  while(l2 > eps) {
    // keep a temporary copy of u
    for(i=1; i<=num_local_lines; i++){
      for(j=1; j<N; j++){
        utmp[i][j] = u[i][j];
      }
    }
    MPI_Status status;
    MPI_Request req;
    int top_neighbor = mpi_rank+1;
    if (top_neighbor<mpi_size) {
      MPI_Isend( u[num_local_lines], numpoints, MPI_DOUBLE,
		 top_neighbor, 0, MPI_COMM_WORLD, &req );
      MPI_Recv(utmp[num_local_lines+1], numpoints, MPI_DOUBLE, top_neighbor, 0,
	       MPI_COMM_WORLD, &status );
      MPI_Wait( &req, &status );
    }
    int bottom_neighbor = mpi_rank-1;
    if (bottom_neighbor>=0) {
      MPI_Isend( u[1], numpoints, MPI_DOUBLE,
		 bottom_neighbor, 0, MPI_COMM_WORLD, &req );
      MPI_Recv( utmp[0], numpoints, MPI_DOUBLE, bottom_neighbor,
		0, MPI_COMM_WORLD, &status );
	MPI_Wait( &req, &status );
    }

    double local_l2 = 0.;
    for(i=1; i<=num_local_lines; i++) {
      for(j=1; j<N; j++) {
        // computation of the new step
        u[i][j] = 0.25 * ( utmp[i-1][j] + utmp[i+1][j] + utmp[i][j-1] + utmp[i][j+1] - f[i][j]*h*h);

        // L2 norm of change in u
        local_l2 += sqr(utmp[i][j] - u[i][j]);
      }
    }
    MPI_Allreduce(&local_l2, &l2, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    if (mpi_rank==0 &&  k%100==0) {
        printf("At step %d, l2=%.5g\n", k, sqrt(l2));
    }

    k++;
  } 

  // output
  MPI_Gather( u[1], num_local_lines*numpoints, MPI_DOUBLE,
              global_matrix_memory, num_local_lines*numpoints, MPI_DOUBLE, 0, MPI_COMM_WORLD );
  if (mpi_rank==0) {
    write_to_bmp(numpoints, global_matrix, k, -1., 1.);
  }

  // deallocate the pointers
  free(u);
  free(utmp);
  free(f);
  free(global_matrix);
  free(u_memory);
  free(utmp_memory);
  free(f_memory);
  free(global_matrix_memory);

  MPI_Finalize();

  return 0;
}



void write_to_bmp(int n, double **tmptab, int iter,
                  double minval, double maxval)
{
  unsigned char *img = NULL;
  int i,j;
  int x,y;
  int w,h;
  int padding, filesize, datasize;
  double v,r,g,b;
  char filename[50];
  FILE *f;
  unsigned char bmpfileheader[14] = {'B','M',0,0,0,0,0,0,0,0,54,0,0,0};
  unsigned char bmpinfoheader[40] = {40, 0, 0, 0,
                                      0, 0, 0, 0,
                                      0, 0, 0, 0,
                                      1, 0,24, 0};

  w = n; // width
  h = n; // height

  padding = (4 - (w*3) % 4) % 4;

  datasize = (3*w + padding)*h;
  filesize = 54 + datasize;

  img = (unsigned char *)calloc(datasize, 1);

  for(i = 0; i < w; i++){
    for(j = 0; j < h; j++){
      x = i;
      y = (h -1) - j;

      v = ((tmptab[j][i] - minval)/(maxval - minval));
      r = v * 255; // Red channel
      g = v * 255; // Green channel
      b = v * 255; // Red channel

      r = min(r, 255);
      g = min(g, 255);
      b = min(b, 255);

      img[(x + y*w)*3 + y*padding + 2] = (unsigned char)(r);
      img[(x + y*w)*3 + y*padding + 1] = (unsigned char)(g);
      img[(x + y*w)*3 + y*padding + 0] = (unsigned char)(b);
    }
  }

  bmpfileheader[ 2] = (unsigned char)(filesize      );
  bmpfileheader[ 3] = (unsigned char)(filesize >>  8);
  bmpfileheader[ 4] = (unsigned char)(filesize >> 16);
  bmpfileheader[ 5] = (unsigned char)(filesize >> 24);

  bmpinfoheader[ 4] = (unsigned char)(       w      );
  bmpinfoheader[ 5] = (unsigned char)(       w >>  8);
  bmpinfoheader[ 6] = (unsigned char)(       w >> 16);
  bmpinfoheader[ 7] = (unsigned char)(       w >> 24);
  bmpinfoheader[ 8] = (unsigned char)(       h      );
  bmpinfoheader[ 9] = (unsigned char)(       h >>  8);
  bmpinfoheader[10] = (unsigned char)(       h >> 16);
  bmpinfoheader[11] = (unsigned char)(       h >> 24);
  bmpinfoheader[20] = (unsigned char)(datasize      );
  bmpinfoheader[21] = (unsigned char)(datasize >>  8);
  bmpinfoheader[22] = (unsigned char)(datasize >> 16);
  bmpinfoheader[23] = (unsigned char)(datasize >> 24);


  sprintf(filename, "out_%04d.bmp", iter);
  f = fopen(filename, "w");

  fwrite(bmpfileheader, 1, 14,f);
  fwrite(bmpinfoheader, 1, 40,f);

  fwrite(img, 1, datasize, f);

  fclose(f);

  free(img);
}
