#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include <errno.h>
#define min(x, y) ((x)<(y)?(x):(y))
#define DONT 0

double* gen_matrix(int n, int m);
int mmult(double *c, double *a, int aRows, int aCols, double *b, int bRows, int bCols);
void compare_matrix(double *a, double *b, int nRows, int nCols);

/** 
    Program to multiply a matrix times a matrix using both
    mpi to distribute the computation among nodes and omp
    to distribute the computation among threads.
*/

int main(int argc, char* argv[])
{
  int nrows, ncols;
  double *aa;	/* the A matrix */
  double *bb;	/* the B matrix */
  double *cc1;	/* A x B computed using the omp-mpi code you write */
  double *cc2;	/* A x B computed using the conventional algorithm */
  int myid, numprocs;
  double starttime, endtime;
  MPI_Status status;
  /* insert other global variables here */






  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
/*  if (argc > 1) {
    nrows = atoi(argv[1]);
    ncols = nrows;
    if (myid == 0) {
      // Master Code goes here



      aa = gen_matrix(nrows, ncols);
      bb = gen_matrix(ncols, nrows);
      cc1 = malloc(sizeof(double) * nrows * nrows);  DON'T NEED THIS */


	if (argc > 2){ // user successfully passed in two file names for two matrices
		if (myid == 0){
 			FILE *a_data; // file ptr for a mtx data
			FILE *b_data; // file ptr for b mtx data
			if ((a_data = fopen(argv[1], "r"))==NULL){
				// failed to open file for a 
				printf("Failed to open matrix a file. errno: %d", errno);
				//tell workers to quit via MPI_send
				exit(0);
			}
			if ((b_data = fopen(argv[2], "r"))==NULL){
				// failed to open file for b
				printf("Failed to open matix b file. erno: %d", errno);
				// tell workers to quit
				for(int i = 0; i < numprocs; i++){
					MPI_send(aa, 0, double, i+1, DONT, MPI_COMM_WORLD);
				}
				exit(0);
			}

		}
	}else{
		printf("Please execute again with two file name arguments (two matrices).");
		exit(0);
	}

  starttime = MPI_Wtime();
      /* Insert your master code here to store the product into cc1 */







      endtime = MPI_Wtime();
      printf("%f\n",(endtime - starttime));
      cc2  = malloc(sizeof(double) * nrows * nrows);
      mmult(cc2, aa, nrows, ncols, bb, ncols, nrows);
      compare_matrices(cc2, cc1, nrows, nrows);
    } else {
      // Slave Code goes here
	// FIRST NEED TO RECEIVE MESSAGE FROM MANAGER TO  SEE IF THEY NEED TO QUIT AUTOMATICALLY
	// i.e. WHEN status.MPI_TAG == DONT
    }
  } else {
    fprintf(stderr, "Usage matrix_times_vector <size>\n");
  }
  MPI_Finalize();
  return 0;
}
