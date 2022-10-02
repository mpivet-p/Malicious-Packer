#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int	check_file(char *filepath)
{
	Elf64_Ehdr	ehdr;
	int			fd;

	if ((fd = open(filepath, O_RDONLY)) < 0)
	{
		perror("woody");
		return (1);
	}
	if (read(fd, &ehdr, sizeof(ehdr)) < 0)
	{
		perror("woody");
		close(fd);
		return (1);
	}
    close(fd);
	if (ehdr.e_ident[EI_MAG0] == ELFMAG0	// Checking file's magic number (0x07454c46)
		&& ehdr.e_ident[EI_MAG1] == ELFMAG1
		&& ehdr.e_ident[EI_MAG2] == ELFMAG2
		&& ehdr.e_ident[EI_MAG3] == ELFMAG3)
	{
		if (ehdr.e_ident[EI_VERSION] == EV_CURRENT
			&& ehdr.e_ident[EI_DATA] == ELFDATA2LSB
			&& ehdr.e_ident[EI_CLASS] == ELFCLASS64)
			return (0);
	}
	fprintf(stderr, "woody: Invalid file.\n");
	return (1);
}