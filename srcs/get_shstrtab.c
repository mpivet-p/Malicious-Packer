#include <elf.h>

int		get_shstrtab_content(void *file, Elf64_Ehdr *ehdr, char **ptr)
{
	Elf64_Shdr	*shdr;

	shdr = (Elf64_Shdr*)(file + ehdr->e_shoff + (sizeof(Elf64_Shdr) * ehdr->e_shstrndx));
	*ptr = (char*)(file + shdr->sh_offset);
	return (0);
}
