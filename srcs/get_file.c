#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>

int	get_file(const char *filepath, void **file, size_t *fsize)
{
	int			fd;

	if ((fd = open(filepath, O_RDONLY)) < 0)
	{
		perror("woody");
		return (1);
	}
	*fsize = lseek(fd, 0, SEEK_END);

	lseek(fd, 0, SEEK_SET);
	*file = mmap(NULL, *fsize, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (*file == MAP_FAILED)
	{
		perror("woody");
		return (1);
	}

	return (0);
}
