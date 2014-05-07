#include <unistd.h>
#include <iostream>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

using namespace std;

int main(int argc , char* args[])
{
	if(argc != 4)
		return -1;
	
	char fileName[20];
	memset(fileName, 0, 20);
	long address,size;
	
	sprintf(fileName,"%s",args[1]);
	
	address = atol(args[2]);
	size = atol(args[3]);
	
	printf("address is %ld , size is %ld\n",address , size);
	if(size <= 0 || address < 0)
	{
		printf("address or size is error\n");
		return -1;
	}

	int fd = open(fileName , O_RDWR);
	if(fd == -1 )
	{
		printf("file %s open error\n",fileName);
		return -1;
	}
	
	void* pAddress = mmap(0 , size , PROT_READ | PROT_WRITE , MAP_SHARED ,fd , address);
	
	if(pAddress == 0)
	{
		printf("mmap error \n");
		close(fd);
		return -1;
	}
	
	char* pBuf = (char*)pAddress;
	
	long i = -1;
	while(++i < size)
	{	
		char c = pBuf[i];
		cout<<c;
	}
	
	cout<<endl;

	if(munmap(pAddress , size) == -1)
	{
		printf("munmap error\n");
	}
	
	close(fd);
	return 0;
}
