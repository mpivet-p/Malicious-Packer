#include <elf.h>
#include <sys/mman.h>
#include <stdio.h>

// Returns the virtual address of the first segment of type == PT_LOAD
uint64_t	get_image_base(void *file)
{
	Elf64_Ehdr	*ehdr;
	Elf64_Phdr	*phdr;

	ehdr = (Elf64_Ehdr*)file;
	
	for (int i = 0; i < ehdr->e_phnum; i++)
	{
		phdr = (Elf64_Phdr*)(file + ehdr->e_phoff + (sizeof(Elf64_Phdr) * i));
		if (phdr->p_type == PT_LOAD)
		{
			printf("addr: 0x%lx\n", phdr->p_vaddr);
			return (phdr->p_vaddr);
		}
	}
	return (0);
}

//DEBUG FUNCTION
int		get_shstrtab_content(void *file, Elf64_Ehdr *ehdr, char **ptr)
{
	Elf64_Shdr	*shdr;

	shdr = (Elf64_Shdr*)(file + ehdr->e_shoff + (sizeof(Elf64_Shdr) * ehdr->e_shstrndx));
	*ptr = (char*)(file + shdr->sh_offset);
	return (0);
}

int		is_pt_load(void *file, Elf64_Ehdr *ehdr, uint64_t offset, uint64_t size)
{
	Elf64_Phdr	*phdr;


	for (int i = 0; i < ehdr->e_phnum; i++)
	{
		phdr = (Elf64_Phdr*)(file + ehdr->e_phoff + i * sizeof(Elf64_Phdr));
		if (offset >= phdr->p_offset
			&& (offset + size) < (phdr->p_offset + phdr->p_filesz))
		{
			return (phdr->p_type == PT_LOAD);
		}
	}
	return (0);
}

int		enough_space_for_payload(void *file, Elf64_Shdr *shdr)
{
	char *content;

	content = (char*)(file + shdr->sh_offset);
	for (int i = shdr->sh_size; i > 0; i--)
	{
		if (content[i - 1] != 0)
			return (shdr->sh_size - i >= 80);
	}
	return (0);
}

void	iterate_over_section_headers(void *file)
{
	Elf64_Ehdr	*ehdr;
	Elf64_Shdr	*shdr;
	char		*shstrtab_content;

	ehdr = (Elf64_Ehdr*)file;
	get_shstrtab_content(file, ehdr, &shstrtab_content);

	for (int i = 0; i < ehdr->e_shnum; i++)
	{

		shdr = (Elf64_Shdr*)(file + ehdr->e_shoff + (i * sizeof(Elf64_Shdr)));
		printf("addr: %lx, offset: %lx, size: %ld, %s", shdr->sh_addr\
		, shdr->sh_offset, shdr->sh_size, shstrtab_content + shdr->sh_name);
		if (shdr->sh_flags & SHF_EXECINSTR)
			printf(" EXEC");
		if (shdr->sh_size >= 80 && is_pt_load(file, ehdr, shdr->sh_offset, shdr->sh_size) == 1)
			printf(" %d", enough_space_for_payload(file, shdr));
		printf("\n");
	}
}

int		woody(char *file_name, void *file, size_t fsize)
{
	(void)file_name;
	get_image_base(file);
	iterate_over_section_headers(file);
	munmap(file, fsize);
	return (0);
}