#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "cdata_ioctl.h"

int main(void)
{
	int fd;

	fd = open("/dev/cdata", O_RDWR);
	ioctl(fd,CDATA_EMPTY);
	ioctl(fd,CDATA_SYNC);
	close(fd);
}
