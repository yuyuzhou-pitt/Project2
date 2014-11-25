#include<stdio.h>

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

void printarray(int* x, int l)
{
	int i;
	for(i=0; i<l; i++)
	{
		printf("%d ",x[i]);
	}
	printf("\n");
}

void printmatrix(int* x, int rows, int cols)
{
	int i,j;
	for(i=0; i<rows; i++)
	{
		for(j=0; j<cols; j++)
		{
			printf("%d ",x[i*cols+j]);
		}
		printf("\n");
	}
}

int main()
{
	
	int a[2][2]={1,2,3,4};
	int b[2][1]={1,1};
	int c[2][1];
	int m,n,l;
	m=2; n=2; l=1;
	Multiply(a,b,c,m,n,l);
	printf("Multiplication:\n");
	printmatrix(c,m,l);
	
	int d[5]={5,3,2,4,1};
	Sort(d, sizeof(d)/sizeof(d[0]));
	printf("Sorting:\n");
	printarray(d, sizeof(d)/sizeof(d[0]));
	
	int min=Min(d, sizeof(d)/sizeof(d[0]));
	int max=Max(d, sizeof(d)/sizeof(d[0]));
	printf("Min:%d; Max:%d\n",min,max);
	return 0;
}
