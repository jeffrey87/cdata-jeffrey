#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

int main(void)
{
	int fd;
	int i;
    unsigned char *fb;

	fd = open("/dev/cdata", O_RDWR);

	fb = (unsigned char *)mmap(0, 240*320*4, PORT_READ|PORT_WRITE, MAP_SHARED, fd, 0);

	for (i = 0 ; i < 500; i++)
	{
		*fb = 0xff; fb++;
		*fb = 0x00; fb++;
		*fb = 0x00; fb++;
		*fb = 0x00; fb++;
	}
	

	close(fd);
}





