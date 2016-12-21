#include<stdio.h>
#include<stdint.h>


int test_warn(void)
{
	int a = 0;

	int *b = NULL;


	b = (int *)(void *)*(int *)a;
	printf("b=%p\n", b);



	return 0;
}
