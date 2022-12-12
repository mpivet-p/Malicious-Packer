#include <elf.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <woody.h>
#include <string.h>

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

Elf64_Shdr	*get_last_section(void *file, uint64_t min, uint64_t max)
{
	Elf64_Ehdr	*ehdr;
	Elf64_Shdr	*shdr;
	Elf64_Shdr	*last_shdr = NULL;
	char		*shstrtab_content;

	ehdr = (Elf64_Ehdr*)file;
	get_shstrtab_content(file, ehdr, &shstrtab_content);
	for (int i = 0; i < ehdr->e_shnum; i++)
	{
		shdr = (Elf64_Shdr*)(file + ehdr->e_shoff + (i * sizeof(Elf64_Shdr)));
		if (shdr->sh_offset >= min && (shdr->sh_offset + shdr->sh_size) <= max)
		{
			if (!(last_shdr) || shdr->sh_offset > last_shdr->sh_offset)
				last_shdr = shdr;
		}
	}
	printf("Last section: %s\n", shstrtab_content + last_shdr->sh_name);
	return (last_shdr);
}

void	write_file(void *file, size_t fsize)
{
	int fd;
	
	if ((fd = open("woody", O_RDWR | O_CREAT | O_TRUNC, 755)) >= 0)
	{
		write(fd, file, fsize);
	}
    close(fd);
}

void	increase_segment_size(Elf64_Phdr *phdr, size_t sizediff)
{
	phdr->p_filesz += sizediff;
	phdr->p_memsz += sizediff;
}

void	copy_payload(void *file, Elf64_Shdr *shdr, uint64_t old_entry)
{
	unsigned char	payload[] =  PAYLOAD_OPCODES;
	void	*ptr;

	ptr = (void*)(file + shdr->sh_offset + shdr->sh_size);
	memcpy(ptr, payload, PAYLOAD_SIZE);

	ptr = (void*)((file + shdr->sh_offset + shdr->sh_size) + PAYLOAD_SIZE - 18);
	*(uint32_t*)(ptr) = old_entry;

//	// Set key
//	ptr = (void*)((file + shdr->sh_offset + shdr->sh_size) + PAYLOAD_SIZE - 18);
//	*(uint32_t*)(ptr) = old_entry;

//	// Set begin
//	ptr = (void*)((file + shdr->sh_offset + shdr->sh_size) + PAYLOAD_SIZE - 18);
//	*(uint32_t*)(ptr) = old_entry;
}

uint64_t	get_payload_addr(Elf64_Phdr *phdr, Elf64_Shdr *shdr)
{
	uint64_t base_address;

	base_address = phdr->p_vaddr - phdr->p_offset;
	return (base_address + shdr->sh_offset + shdr->sh_size);
}

int	generate_key(void)
{
	size_t	len;
	int		fd;
	int		key;

	if ((fd = open("/dev/urandom", O_RDONLY)) < 0
		|| (len = read(fd, &key, 4)) != 4)
	{
		fprintf(stderr, "woody: key generation failed!");
		exit(1);
	}
	close(fd); // Need to close if read fails too
	return (key);
}

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

void	inject(void *file, Elf64_Ehdr *ehdr, Elf64_Phdr *phdr)
{
	Elf64_Shdr	*shdr;
	uint32_t	jmp_addr;
	uint64_t	payload_addr;

	shdr = get_last_section(file, phdr->p_offset, phdr->p_offset + phdr->p_filesz);
	payload_addr = get_payload_addr(phdr, shdr);
	//Might need to make it relative to the segment address (if the .text section is on another segment)
    jmp_addr = (uint32_t)(ehdr->e_entry - (payload_addr + PAYLOAD_SIZE - 14));
    printf("\njmp 0x%x (relative: %d)\n", jmp_addr, (int)jmp_addr); //Debug

    ehdr->e_entry = payload_addr;
	copy_payload(file, shdr, jmp_addr);
	increase_segment_size(phdr, PAYLOAD_SIZE);
}

int		woody(char *file_name, void *file, size_t fsize)
{
	Elf64_Phdr	*phdr;
	Elf64_Ehdr	*ehdr;

	ehdr = (Elf64_Ehdr*)file;
	if ((phdr = find_cave_segment(file, ehdr)))
	{
		printf("0x%x\n", generate_key());
		encrypt_text_section(file);
		inject(file, ehdr, phdr);
		write_file(file, fsize);
	}
	else
	{
		fprintf(stderr, "woody: error with file %s!\n", file_name);
	}
	munmap(file, fsize);
	return (0);
}
