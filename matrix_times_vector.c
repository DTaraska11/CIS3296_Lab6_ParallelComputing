#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define min(x, y) ((x)<(y)?(x):(y))

int main(int argc, char* argv[])
{
    int nrows, ncols;
    double *aa, *b, *c;
    double *buffer, ans;
    double *times;
    double total_times;
    int run_index;
    int nruns;
    int myid, master, numprocs;
    double starttime, endtime;
    MPI_Status status;
    int i, j, numsent, sender;
    int anstype, row;
    srand(time(0));
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    if (argc > 1) {
        nrows = atoi(argv[1]);
        ncols = nrows;
        aa = (double*)malloc(sizeof(double) * nrows * ncols);
        b = (double*)malloc(sizeof(double) * ncols);
        c = (double*)malloc(sizeof(double) * nrows);
        buffer = (double*)malloc(sizeof(double) * ncols);
        master = 0;
        if (myid == master) {
            // Master Code goes here
            for (i = 0; i < nrows; i++) {
                for (j = 0; j < ncols; j++) {
                    aa[i*ncols + j] = (double)rand()/RAND_MAX;
                }
            }
            
            starttime = MPI_Wtime();
            numsent = 0;
            // BROADCASTS b TO ALL PROCESSES
            MPI_Bcast(b, ncols, MPI_DOUBLE, master, MPI_COMM_WORLD);
            
            //manager begins sending each individual process a row of aa to work on
            for (i = 0; i < min(numprocs-1, nrows); i++) {
                for (j = 0; j < ncols; j++) {
                    buffer[j] = aa[i * ncols + j];
                }
                MPI_Send(buffer, ncols, MPI_DOUBLE, i+1, i+1, MPI_COMM_WORLD);
                numsent++;
            }
            
            // manager waits to received answers back from each process
            for (i = 0; i < nrows; i++) {
                MPI_Recv(&ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG,
                         MPI_COMM_WORLD, &status);
                sender = status.MPI_SOURCE;
                anstype = status.MPI_TAG;
                c[anstype-1] = ans; // manually inserts answer to corresponding entry in c
                
                // sends more "slices" of aa IFF dimension of aa is more than number of processes
                if (numsent < nrows) {
                    for (j = 0; j < ncols; j++) {
                        buffer[j] = aa[numsent*ncols + j];
                    }
                    MPI_Send(buffer, ncols, MPI_DOUBLE, sender, numsent+1,
                             MPI_COMM_WORLD);
                    numsent++;
                } else {
                    MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, sender, 0, MPI_COMM_WORLD);
                }
            }
            endtime = MPI_Wtime();
            printf("%f\n",(endtime - starttime));
        } else {
            // Worker Code goes here
            MPI_Bcast(b, ncols, MPI_DOUBLE, master, MPI_COMM_WORLD);
            if (myid <= nrows) {
                while(1) {
                    MPI_Recv(buffer, ncols, MPI_DOUBLE, master, MPI_ANY_TAG,
                             MPI_COMM_WORLD, &status);
                    if (status.MPI_TAG == 0){
                        break;
                    }
                    row = status.MPI_TAG;
                    ans = 0.0;
                    for (j = 0; j < ncols; j++) {
                        ans += buffer[j] * b[j];
                    }
                    MPI_Send(&ans, 1, MPI_DOUBLE, master, row, MPI_COMM_WORLD);
                }
            }
        }
    } else {
        fprintf(stderr, "Usage matrix_times_vector <size>\n");
    }
    MPI_Finalize();
    return 0;
}
