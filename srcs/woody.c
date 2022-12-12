#include <elf.h>
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <woody.h>
#include <string.h>


void	dump(void *file, uint64_t offset, size_t size)
{
	void *ptr = file + offset;

	printf("%p\n", ptr);
	for (size_t i = 0; i < size; i++)
	{
		if (i % 16 == 0)
		{
			printf("%016lx\t", offset + i);
		}
		printf("%02x ", *(uint8_t*)(ptr + i));
		if (i % 16 == 15 || i + 1 == size)
			printf("\n");
	}
}

// Returns the virtual address of the first segment of type == PT_LOAD
//uint64_t	get_image_base(void *file)
//{
//	Elf64_Ehdr	*ehdr;
//	Elf64_Phdr	*phdr;
//
//	ehdr = (Elf64_Ehdr*)file;
//	
//	for (int i = 0; i < ehdr->e_phnum; i++)
//	{
//		phdr = (Elf64_Phdr*)(file + ehdr->e_phoff + (sizeof(Elf64_Phdr) * i));
//		if (phdr->p_type == PT_LOAD)
//		{
//			return (phdr->p_vaddr);
//		}
//	}
//	return (0);
//}

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

void	addr_to_str(unsigned char *str, char *key, uint32_t addr)
{
	size_t	i = 0;
	size_t	j = 0;

	while (i < 4)
	{
		if (str[j] == key[i])
			i++;
		else
			i = 0;
		j++;
	}
	if (i != 4)
		return ;
	j -= 4;
	str[j + 3] = (unsigned char)((addr >> 24) & 0xFF);
	str[j + 2] = (unsigned char)((addr >> 16) & 0xFF);
	str[j + 1] = (unsigned char)((addr >> 8) & 0xFF);
	str[j] = (unsigned char)(addr & 0xFF);
}

void	insert_payload(void *file, Elf64_Shdr *shdr, uint64_t old_entry)
{
	unsigned char	payload[] =  PAYLOAD_OPCODES;
	void	*ptr;

//	addr_to_str((unsigned char*)payload, "\xfb\xff\xff\xff", old_entry); // Setting up the jump to the old entry
(void)old_entry;
	ptr = (void*)(file + shdr->sh_offset + shdr->sh_size);
	memcpy(ptr, payload, PAYLOAD_SIZE);
	ptr = (void*)(ptr + PAYLOAD_SIZE - 18);
	printf("%x\n", *(uint32_t*)(ptr));
	*(uint32_t*)(ptr) = old_entry;
}

uint64_t	get_payload_addr(Elf64_Phdr *phdr, Elf64_Shdr *shdr)
{
	uint64_t base_address;

	base_address = phdr->p_vaddr - phdr->p_offset;
	return (base_address + shdr->sh_offset + shdr->sh_size);
}

uint64_t	set_entrypoint(void *file, Elf64_Ehdr *ehdr, uint64_t payload_addr)
{
	Elf64_Shdr	*init_array;
//	Elf64_Shdr	*shdr;
//	Elf64_Rela	*rela;
//	uint64_t	old;

	init_array = get_section_header(file, ".init_array");
	// printf(".init_array:\n");
	// dump(file, init_array->sh_offset, init_array->sh_size);
//	if (ehdr->e_type == ET_DYN)
//	{
//		printf("Binary is Shared object file\n");
//		if (!(shdr = get_section_header(file, ".rela.dyn")))
//		{
//			fprintf(stderr, "woody: missing section (.rela.dyn), cannot procede!\n");
//			return (0);
//		}
//		for (size_t i = 0; i < (shdr->sh_size / sizeof(Elf64_Rela)); i++)
//		{
//			rela = (Elf64_Rela*)(file + shdr->sh_offset + (sizeof(Elf64_Rela) * i));
//			if (init_array->sh_addr == rela->r_offset)
//			{
//				dump(file, shdr->sh_offset + (sizeof(Elf64_Rela) * i), sizeof(Elf64_Rela));
//				old = rela->r_addend;
//				printf("rela.r_addend: %lx\n", rela->r_addend);
//				rela->r_addend = payload_addr;
//				break ;
//			}
//		}
//	}
	// memcpy(&old, file + init_array->sh_offset, 8);
	// memcpy(file + init_array->sh_offset, &payload_addr, 8);
	// dump(file, init_array->sh_offset, 8);
	ehdr->e_entry = payload_addr;
	return (42);
}

uint64_t test_entry(void *file)
{
	Elf64_Shdr	*text_section;

	text_section = get_section_header(file, ".text");
    return (text_section->sh_addr);
}

//Need refacto
void	iterate_over_program_headers(void *file)
{
	Elf64_Ehdr	*ehdr;
	Elf64_Phdr	*phdr;
	Elf64_Shdr	*shdr;
	Elf64_Shdr	*text_shdr;
	uint32_t	old_entrypoint;
	uint64_t	payload_addr;
	size_t		whitespaces;

	ehdr = (Elf64_Ehdr*)file;
	for (int i = 0; i < ehdr->e_phnum; i++)
	{
		phdr = (Elf64_Phdr*)(file + ehdr->e_phoff + (i * sizeof(Elf64_Phdr)));
		if (phdr->p_type == PT_LOAD)
		{
			whitespaces = phdr->p_align - (phdr->p_filesz % phdr->p_align);
			if (whitespaces > PAYLOAD_SIZE && phdr->p_flags & PF_X)
			{
				old_entrypoint = test_entry(file);
				shdr = get_last_section(file, phdr->p_offset, phdr->p_offset + phdr->p_filesz);
				payload_addr = get_payload_addr(phdr, shdr);
                old_entrypoint = (uint32_t)(ehdr->e_entry - (payload_addr + PAYLOAD_SIZE - 19));
                //old_entrypoint = (uint32_t)((ehdr->e_entry + phdr->p_vaddr) - (payload_addr + PAYLOAD_SIZE));
                printf("\njmp 0x%x\n", old_entrypoint);
				text_shdr = get_section_header(file, ".text");
                printf("e_entry: 0x%lx .text offset: 0x%lx p_vaddr: 0x%lx dist: 0x%lx\n", ehdr->e_entry, text_shdr->sh_addr, phdr->p_vaddr, (payload_addr + PAYLOAD_SIZE));
                printf("\nint %d\n", (int)old_entrypoint);
                ehdr->e_entry = payload_addr;
				insert_payload(file, shdr, old_entrypoint);
				increase_segment_size(phdr, PAYLOAD_SIZE);
				return ;
			}
		}
	}
}

int		woody(char *file_name, void *file, size_t fsize)
{
	(void)file_name;
	iterate_over_program_headers(file);
	write_file(file, fsize);
	munmap(file, fsize);
	return (0);
}
