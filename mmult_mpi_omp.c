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
    int a_nrows, a_ncols, b_nrows, b_ncols;
    double *aa;    /* the A matrix */
    double *bb;    /* the B matrix */
    double *cc1;    /* A x B computed using the omp-mpi code you write */
    double *cc2;    /* A x B computed using the conventional algorithm */
    int myid, numprocs;
    double starttime, endtime;
    MPI_Status status;
    int continue_mult = 1;
    
    
    
    
    
    
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
        if (myid==0){
            FILE *a_data; // file ptr for a mtx data
            FILE *b_data; // file ptr for b mtx data
            if ((a_data = fopen(argv[1], "r"))==NULL){
                // failed to open file for a
                printf("Failed to open matrix a file. errno: %d", errno);
                continue_mult = 0;
            } else if ((b_data = fopen(argv[2], "r"))==NULL){
                // failed to open file for b
                printf("Failed to open matix b file. erno: %d", errno);
                continue_mult = 0;
            }
            
            // MANAGER CHECKS THAT MATRICES FROM FILES MEET DIMENSION REQUIREMENTS
                 // get nrows and ncols for A and B from first lines of respective files
            /*fscanf(a_data, "rows(%d) cols(%d)\n", &a_nrows, &a_ncols);
            fscanf(b_data, "rows(%d) cols(%d)\n", &b_nrows, &b_ncols);
             THIS METHOD OF fscanf DOES NOT SUCCESSFULLY EXTRACT THE VALUES FOR ROWS & COLS
             - MUST FIX
             
             */
            
            if(a_ncols != b_nrows){// dimension requirements not satisfied
                continue_mult = 0;
            }
            
            if (continue_mult){
            // MANAGER FORMATS MATRICES aa & bb
                aa = (double*)malloc(sizeof(double) * a_nrows * a_ncols);
                bb = (double*)malloc(sizeof(double) * b_nrows * b_ncols);
                cc1 = (double*)malloc(sizeof(double) * a_nrows * b_ncols);
                
                // format aa
                /*for(int i = 0; i < a_nrows; i++){
                    for(int j = 0; j< a_ncols; j++){
                        
                    }
                }*/
                
                //format bb
                
                
                // start timer
                fclose(a_data), fclose(b_data); // close file ptrs
                starttime = MPI_Wtime();
        
    /* Insert your master code here to store the product into cc1 */
                
                //manager broadcasts bb to workers
                MPI_Bcast(bb, (b_nrows*b_ncols), MPI_DOUBLE, 0, MPI_COMM_WORLD);
                
                //manager begins sending each individual process a row of aa to work on
                double *buffer = (double*)malloc(sizeof(double) * a_ncols);
                for (i = 0; i < min(numprocs-1, a_nrows); i++) {
                    for (j = 0; j < a_ncols; j++) {
                        buffer[j] = aa[i * a_ncols + j];
                    }
                    MPI_Send(buffer, a_ncols, MPI_DOUBLE, i+1, i+1, MPI_COMM_WORLD);
                    numsent++;
                }
                
                // manager waits to receive answers back from each process
                for (i = 0; i < nrows; i++) {
                    MPI_Recv(&ans, 1, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG,
                             MPI_COMM_WORLD, &status);
                    sender = status.MPI_SOURCE;
                    anstype = status.MPI_TAG;
                    cc1[anstype-1] = ans; // manually inserts answer to corresponding entry in cc
                    
                    // sends more "slices" of aa IFF dimension of aa is more than number of processes
                    if (numsent < nrows) {
                        for (j = 0; j < a_ncols; j++) {
                            buffer[j] = aa[numsent*a_ncols + j];
                        }
                        MPI_Send(buffer, a_ncols, MPI_DOUBLE, sender, numsent+1,
                                 MPI_COMM_WORLD);
                        numsent++;
                    } else {
                        MPI_Send(MPI_BOTTOM, 0, MPI_DOUBLE, sender, 0, MPI_COMM_WORLD);
                    }
                }
                endtime = MPI_Wtime();
                printf("%f\n",(endtime - starttime));
                cc2  = malloc(sizeof(double) * nrows * nrows);
                mmult(cc2, aa, nrows, ncols, bb, ncols, nrows);
                compare_matrices(cc2, cc1, nrows, nrows);
            }else{ // tell workers to quit via MPI_send with tag "DONT"
                /*for(int i = 0; i < numprocs; i++){
                    MPI_send(aa, 0, double, i+1, DONT, MPI_COMM_WORLD);
                }*/
                fclose(a_data), fclose(b_data); // close file ptrs
            }
        }else{
            // Worker Code goes here
            // FIRST NEED TO RECEIVE MESSAGE FROM MANAGER TO  SEE IF THEY NEED TO QUIT AUTOMATICALLY
            // i.e. WHEN status.MPI_TAG == DONT
            
            /*MPI_Recv(aa, 0, MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status); // check if file i/o errors occurred
            if (status.MPI_TAG != DONT){ // no file i/o errors occurred
                MPI_Bcast(bb, (b_nrows*b_ncols), MPI_DOUBLE, 0, MPI_COMM_WORLD); // receive broadcasted B mtx from manager
                if (myid <= a_nrows) { // check that process corresponds to a row of A
                    while(1) {
                        MPI_Recv(buffer, a_ncols, MPI_DOUBLE, 0, MPI_ANY_TAG,
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
            }*/
        
            
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
    }else{ // user failed to provide two matrices-- tell workers to exit and exit
        if (myid==0){
            printf("Did not include matrix data (file) for both matrices to be multiplied. Try again.");
        }
    }
    
    MPI_Finalize();
    return 0;
}
