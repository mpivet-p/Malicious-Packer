#include "woody.h"
#include <stdio.h>

int				main(int argc, char **argv)
{
	void	*file_content = NULL;
	size_t	file_size = 0;
	int		ret = 0;

	if (argc == 2 && check_file(argv[1]) == 0)
	{
		if (get_file(argv[1], &file_content, &file_size) != 0
			|| woody(argv[1], file_content, file_size) != 0)
			ret = 1;
	}
	return (ret);
}
