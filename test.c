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
	if (pid != 0) {
		write(fd, "ABCDE", 5);
	} 
	else {
		write(fd, "12345", 5);
	}
	ioctl(fd,CDATA_SYNC,0);
	ioctl(fd,CDATA_EMPTY,0);
	close(fd);
}
