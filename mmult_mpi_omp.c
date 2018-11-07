#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#define min(x, y) ((x)<(y)?(x):(y))

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
    int a_nrows, a_ncols, b_ncols, b_nrows;
    double *aa;    /* the A matrix */
    double *bb;    /* the B matrix */
    double *cc1;    /* A x B computed using the omp-mpi code you write */
    double *cc2;    /* A x B computed using the conventional algorithm */
    double *buffer, ans;
    int myid, numprocs, i, j, numsent, sender, anstype, row;
    srand(time(0));
    double starttime, endtime;
    MPI_Status status;
    /* insert other global variables here */
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    if (argc > 1) {
        b_ncols = a_nrows = atoi(argv[1]);
        b_nrows = a_ncols = a_nrows+2;
        if (myid == 0) {
            // Master Code goes here
            aa = malloc(sizeof(double)*a_nrows*a_ncols);
            bb = malloc(sizeof(double)*b_nrows*b_ncols);
         
            for (i = 0; i < a_nrows; i++) {
              for (j = 0; j < a_ncols; j++) {
                aa[i*a_ncols + j] = (double)rand()/RAND_MAX;
              }
            }
         
           for (i = 0; i < b_nrows; i++) {
              for (j = 0; j < b_ncols; j++) {
                bb[i*b_ncols + j] = (double)rand()/RAND_MAX;
              }
            }
           // aa = gen_matrix(a_nrows, a_ncols);
            //bb = gen_matrix(b_nrows, b_ncols);
            cc1 = (double*)malloc(sizeof(double) * a_nrows * b_ncols);
            starttime = MPI_Wtime();
            numsent = 0;
            //manager broadcasts bb to workers
            MPI_Bcast(bb, (b_nrows*b_ncols), MPI_DOUBLE, 0, MPI_COMM_WORLD);
            
            //manager begins sending each individual process a row of aa to work on
            buffer = (double*)malloc(sizeof(double) * a_ncols);
            for (i = 0; i < min(numprocs-1, a_nrows); i++) {
                for (j = 0; j < a_ncols; j++) {
                    buffer[j] = aa[i * a_ncols + j];
                }
                MPI_Send(buffer, a_ncols, MPI_DOUBLE, i+1, i+1, MPI_COMM_WORLD);
                numsent++;
            }
            
            // manager waits to receive answers back from each process
            for (i = 0; i < a_nrows; i++) {
                MPI_Recv(&ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG,
                         MPI_COMM_WORLD, &status);
                sender = status.MPI_SOURCE;
                anstype = status.MPI_TAG;
                cc1[anstype-1] = ans; // manually inserts answer to corresponding entry in cc
                
                // sends more "slices" of aa IFF dimension of aa is more than number of processes
                if (numsent < a_nrows) {
                    for (j = 0; j < a_ncols; j++) {
                        buffer[j] = aa[numsent*a_ncols + j];
                    }
                    MPI_Send(buffer, a_ncols, MPI_DOUBLE, sender, numsent+1,
                             MPI_COMM_WORLD);
                    numsent++;
                } else {
                    MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, sender, 0, MPI_COMM_WORLD);
                }
            endtime = MPI_Wtime();
            printf("%f\n",(endtime - starttime));
            cc2  = (double*)malloc(sizeof(double) * a_nrows * b_ncols);
            mmult(cc2, aa, a_nrows, a_ncols, bb, b_ncols, b_nrows);
            compare_matrices(cc2, cc1, a_nrows, b_ncols);
            }
        } else {
            // Slave Code goes here
            // needs review: check that processes only multiply when they're supposed to
            MPI_Bcast(bb, (b_nrows*b_ncols), MPI_DOUBLE, 0, MPI_COMM_WORLD); //receive broadcasted matrix bb
            if (myid <= a_nrows) {
                while(1) {
                    MPI_Recv(buffer, a_ncols, MPI_DOUBLE, 0, MPI_ANY_TAG,
                             MPI_COMM_WORLD, &status);
                    if (status.MPI_TAG == 0){
                        break;
                    }
                    row = status.MPI_TAG;
                    ans = 0.0;
                    for (j = 0; j < a_ncols; j++) {
                        ans += buffer[j] * bb[j*b_ncols+row];
                    }
                    MPI_Send(&ans, 1, MPI_DOUBLE, 0, row, MPI_COMM_WORLD);
                }
            }
        }
    } else {
        fprintf(stderr, "Usage matrix_times_vector <size>\n");
    }
    MPI_Finalize();
    return 0;
}
