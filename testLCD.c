#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main(void)
{
	int fd;
	char pixel[4] = {0x33, 0x00, 0x00, 0x00}; // {B,G,R,dummy} little endian

	fd = open("/dev/cdata", O_RDWR);

	while(1)
	{
		write(fd, pixel, 4);
	}

	close(fd);
}





