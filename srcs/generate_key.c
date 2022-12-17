#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int	generate_key(void)
{
	size_t	len;
	int		fd;
	int		key;

	if ((fd = open("/dev/urandom", O_RDONLY)) >= 0)
	{
		len = read(fd, &key, 4);
		close(fd);
		if (len == 4)
		{
			return (key);
		}
	}
	fprintf(stderr, "woody: key generation failed!");
	exit(1);
}
