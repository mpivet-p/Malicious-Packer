#include "woody.h"
#include <elf.h>
#include <stddef.h>

Elf64_Phdr	*find_cave_segment(void *file, Elf64_Ehdr *ehdr)
{
	Elf64_Phdr	*phdr;
	size_t		whitespaces;

	for (int i = 0; i < ehdr->e_phnum; i++)
	{
		phdr = (Elf64_Phdr*)(file + ehdr->e_phoff + (i * sizeof(Elf64_Phdr)));
		if (phdr->p_type != PT_LOAD || !(phdr->p_flags & PF_X))
			continue ;

		whitespaces = phdr->p_align - (phdr->p_filesz % phdr->p_align);
		if (whitespaces > PAYLOAD_SIZE)
		{
			return (phdr);
		}
	}
	return (NULL);
}
