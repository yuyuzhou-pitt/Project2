#include<stdio.h>
#include<stdlib.h>

int* Multiply(int* a, int* b, int* c, int m, int n, int l)
{
	int i,j,k;
	for(i=0;i<m;i++)
	{
		for(k=0;k<l;k++)
		{
			c[i*l+k]=0;
			for(j=0;j<n;j++)
			{
				c[i*l+k] += a[i*n+j]*b[j*l+k];
			}
		}
	}
	return c;
}

int cmp(const void* a, const void *b)
{
	return *(int *)a - *(int *)b;
}

void Sort(int* x, int l)
{
	qsort(x, l, sizeof(x[0]), cmp);
}

int	Min(int* x, int l)
{
	int i,min=999999999;
	for(i=0; i<l; i++)
		if(x[i]<min) min=x[i];
	return min;
}

int Max(int* x, int l)
{
	int i,max=-999999999;
	for(i=0; i<l; i++)
		if(x[i]>max) max=x[i];
	return max;
}

void printarray(FILE* out, int* x, int l)
{
	int i;
	for(i=0; i<l; i++)
	{
		fprintf(out,"%d ",x[i]);
	}
	fprintf(out,"\n");
}

void printmatrix(FILE* out, int* x, int rows, int cols)
{
	int i,j;
	for(i=0; i<rows; i++)
	{
		for(j=0; j<cols; j++)
		{
			fprintf(out,"%d ",x[i*cols+j]);
		}
		fprintf(out,"\n");
	}
}
