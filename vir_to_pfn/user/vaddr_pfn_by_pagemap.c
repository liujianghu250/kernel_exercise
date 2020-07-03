#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>

long get_pfn(char *filename, unsigned long vaddr){
	int pageSize = getpagesize();//调用此函数获取系统设定的页面大小
	unsigned long v_offset = vaddr / pageSize * sizeof(uint64_t);
	uint64_t item;
	int fd = open(filename, O_RDONLY);
	
	if(fd == -1){
		printf("open %s failed\n",filename);
		return -1;
	}
	if(lseek(fd, v_offset, SEEK_SET) == -1){
		printf("sleek failed\n");
		return -1;
	}
	if(read(fd, &item, sizeof(uint64_t)) != sizeof(uint64_t)){
		printf("read failed\n");
		return -1;
	}
	printf("error might be here\n");
	long pfn = (((uint64_t)1 << 55) - 1) & item;
	return pfn;
}

int main(){
	char pid[20];
	unsigned long vaddr;
	char filename[80];
	printf("Please input the pid and the virtual address\n");
	scanf("%s%lx",pid, &vaddr);
	printf("pid: %s\tvaddr: %lx\n",pid,vaddr);
	strcpy(filename, "/proc/");
	strcat(filename, pid);
	strcat(filename, "/pagemap");
	long pfn = get_pfn(filename, vaddr);
	if(pfn == -1){
		printf("the virtual address is bad.\n");
	}else{
		printf("pfn = 0x%lx\n",pfn);
	}
	return 0;
}
