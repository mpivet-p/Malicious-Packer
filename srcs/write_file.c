#include <fcntl.h>
#include <unistd.h>

void	write_file(void *file, size_t fsize)
{
	int fd;
	
	if ((fd = open("woody", O_RDWR | O_CREAT | O_TRUNC, 755)) >= 0)
	{
		write(fd, file, fsize);
	}
    close(fd);
}
