#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "cdata_ioctl.h"

int main(void)
{
	int fd;
	
	pid_t pid = fork(); 
	fd = open("/dev/cdata", O_RDWR);
	//fork();
	ioctl(fd,CDATA_EMPTY,0);
	ioctl(fd,CDATA_SYNC,0);
	close(fd);
}
