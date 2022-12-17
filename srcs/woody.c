#include "woody.h"
#include <elf.h>
#include <stdio.h>
#include <sys/mman.h>

int		woody(char *file_name, void *file, size_t fsize)
{
	Elf64_Phdr	*phdr;
	Elf64_Ehdr	*ehdr;
	uint32_t	key;

	ehdr = (Elf64_Ehdr*)file;
	if ((phdr = find_cave_segment(file, ehdr)))
	{
		key = generate_key();
		encrypt_text_section(file, key);
		inject(file, ehdr, phdr, key);
		write_file(file, fsize);
	}
	else
	{
		fprintf(stderr, "woody: error with file %s!\n", file_name);
	}
	munmap(file, fsize);
	return (0);
}
