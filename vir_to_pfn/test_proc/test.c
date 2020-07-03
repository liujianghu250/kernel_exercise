#include <stdio.h>
#include <stdlib.h>

int main()
{
	char *arr = (char *)malloc(100);
	arr[0] = 'a';
	int y = 100;
	printf("In this process, there is an array,\nits begin address is :%x\nthe end address is :%x\n", &arr[0], &arr[99]);
	while(1){
				
	}
	return 0;
}
