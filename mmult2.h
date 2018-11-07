#ifndef mmult2_h
#define mmult2_h

#include <stdio.h>
int mmult(double *c, double *a, int aRows, int aCols, double *b, int bRows, int bCols);
double* gen_matrix(int n, int m);
void compare_matrices(double* a, double* b, int nRows, int nCols);
#endif /* mmult2_h */
