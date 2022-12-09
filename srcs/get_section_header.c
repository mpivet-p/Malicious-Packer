#include "woody.h"
#include <elf.h>
#include <string.h>

Elf64_Shdr	*get_section_header(void *file, char *name)
{
	Elf64_Ehdr	*ehdr;
	Elf64_Shdr	*shdr;
	char		*shstrtab_content;

	ehdr = (Elf64_Ehdr*)file;
	get_shstrtab_content(file, ehdr, &shstrtab_content);

	for (int i = 0; i < ehdr->e_shnum; i++)
	{
		shdr = (Elf64_Shdr*)(file + ehdr->e_shoff + (i * sizeof(Elf64_Shdr)));
		if (strcmp(shstrtab_content + shdr->sh_name, name) == 0)
		{
			return (shdr);
		}
	}
	return (NULL);
}
