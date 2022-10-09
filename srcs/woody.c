#include <elf.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <woody.h>

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

Elf64_Shdr	*get_last_section(void *file, uint64_t min, uint64_t max)
{
	Elf64_Ehdr	*ehdr;
	Elf64_Shdr	*shdr;
	Elf64_Shdr	*last_shdr = NULL;
	// char		*shstrtab_content;

	ehdr = (Elf64_Ehdr*)file;
	// get_shstrtab_content(file, ehdr, &shstrtab_content);
	for (int i = 0; i < ehdr->e_shnum; i++)
	{
		shdr = (Elf64_Shdr*)(file + ehdr->e_shoff + (i * sizeof(Elf64_Shdr)));
		if (shdr->sh_offset >= min && (shdr->sh_offset + shdr->sh_size) <= max)
		{
			// printf("%s\n", shstrtab_content + shdr->sh_name);
			if (!(last_shdr) || shdr->sh_offset > last_shdr->sh_offset)
				last_shdr = shdr;
		}
	}
	// printf("Last section: %s\n", shstrtab_content + last_shdr->sh_name);
	return (last_shdr);
}

size_t	increase_section_size(Elf64_Shdr *shdr, size_t sizediff)
{
	shdr->sh_size += sizediff;
	return (shdr->sh_size - sizediff);
}

void	write_file(void *file, size_t fsize)
{
	int fd;
	
	if ((fd = open("woody", O_RDWR | O_CREAT | O_TRUNC, 755)) >= 0)
	{
		write(fd, file, fsize);
	}
}

void	set_section_executable(Elf64_Shdr *shdr)
{
	shdr->sh_flags |= SHF_EXECINSTR;
}

void	increase_segment_size(Elf64_Phdr *phdr, size_t sizediff)
{
	phdr->p_filesz += sizediff;
	phdr->p_memsz += sizediff;
}

void	insert_payload(void *file, Elf64_Shdr *shdr, uint64_t old_entrypoint)
{
	// mov rax, 0x401195
	// jmp rax
	char	payload[] = "\xb8\x20\x10\x40\x00\xff\xe0";
	void	*ptr;

	(void)old_entrypoint;
	ptr = (void*)(file + shdr->sh_offset + shdr->sh_size + 8);
	for (size_t i = 0; i < 7; i++) 
	{
		((char*)ptr)[i] = payload[i];
	}
}

uint64_t	modify_entrypoint(Elf64_Ehdr *ehdr, Elf64_Phdr *phdr, Elf64_Shdr *shdr)
{
	uint64_t base_address;
	uint64_t old_entrypoint;

	base_address = phdr->p_vaddr - phdr->p_offset;
	old_entrypoint = ehdr->e_entry;
	ehdr->e_entry = base_address + shdr->sh_offset + shdr->sh_size + 8;
	return (old_entrypoint);
}

void	iterate_over_program_headers(void *file)
{
	Elf64_Ehdr	*ehdr;
	Elf64_Phdr	*phdr;
	Elf64_Shdr	*shdr;
	size_t whitespaces;
	uint64_t old_entrypoint;

	ehdr = (Elf64_Ehdr*)file;
	for (int i = 0; i < ehdr->e_phnum; i++)
	{

		phdr = (Elf64_Phdr*)(file + ehdr->e_phoff + (i * sizeof(Elf64_Phdr)));
		if (phdr->p_type == PT_LOAD)
		{
			printf("0x%lx -> PT_LOAD\n", phdr->p_offset);
			whitespaces = phdr->p_align - (phdr->p_filesz % phdr->p_align);
			printf("%zu whitespaces\n", whitespaces);
			if (whitespaces > PAYLOAD_SIZE && phdr->p_flags & PF_X)// && phdr->p_offset == 0x001000)
			{
				shdr = get_last_section(file, phdr->p_offset, phdr->p_offset + phdr->p_filesz);
				old_entrypoint = modify_entrypoint(ehdr, phdr, shdr);
				insert_payload(file, shdr, old_entrypoint);
				increase_section_size(shdr, PAYLOAD_SIZE);
				set_section_executable(shdr);
				increase_segment_size(phdr, PAYLOAD_SIZE);
				return ;
			}
		}
	}
}

int		woody(char *file_name, void *file, size_t fsize)
{
	(void)file_name;
	get_image_base(file);
	//iterate_over_section_headers(file);
	iterate_over_program_headers(file);
	write_file(file, fsize);
	munmap(file, fsize);
	return (0);
}