#ifndef __LIB_MATH_H__
#define __LIB_MATH_H__

int* Multiply(int* a, int* b, int* c, int m, int n, int l);
int cmp(const void* a, const void *b);
void Sort(int* x, int l);
int Min(int* x, int l);
int Max(int* x, int l);
void printarray(FILE* out, int* x, int l);
void printmatrix(FILE* out, int* x, int rows, int cols);

#endif
