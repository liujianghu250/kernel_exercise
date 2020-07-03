#include <stdio.h>
#include <stdlib.h>

int main()
{
	FILE *fp = NULL;
	char *pid = (char *)malloc(11*sizeof(char));
	char *vaddr = (char *)malloc(21*sizeof(char));	
	char *msg = (char *)malloc(1024);

	printf("Please input the pid and the virtual address.\ne.g. >>> 1000 0x1356\n>>>");
	scanf("%s%s",pid,vaddr);
	fp = fopen("/proc/vaddr_pfn","r+");
	
	if(!fp){
		printf("can't open /proc/vaddr_pfn\n");
		return 0;
	}

	if(fprintf(fp,"%s %s",pid,vaddr) < 0){
		printf("Write Failed!\n");
		return 0;
	}	
	if(fgets(msg, 1024, fp) ==NULL){
		printf("Read Failed!\n");
		return 0;
	}
	printf("%s\n",msg);
	return 0;

}
