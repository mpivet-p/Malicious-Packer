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

size_t	prepare_section(Elf64_Shdr *shdr, size_t sizediff)
{
	shdr->sh_size += sizediff;
	shdr->sh_flags |= SHF_EXECINSTR;
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

void	increase_segment_size(Elf64_Phdr *phdr, size_t sizediff)
{
	phdr->p_filesz += sizediff;
	phdr->p_memsz += sizediff;
}

void	addr_to_str(unsigned char *str, char *key, uint64_t addr)
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
	printf("i = %zu   %zu\n", i, j);
	j -= 4;
	str[j + 3] = (unsigned char)((addr >> 24) & 0xFF);
	str[j + 2] = (unsigned char)((addr >> 16) & 0xFF);
	str[j + 1] = (unsigned char)((addr >> 8) & 0xFF);
	str[j] = (unsigned char)(addr & 0xFF);
}

void	insert_payload(void *file, uint64_t new_entry, Elf64_Shdr *shdr, uint64_t old_entry)
{
	//\x41\x44\x44\x52 = ADDR       \x53\x54\x52\x57 = STRW
	//unsigned char	payload[] = "\x57\x56\x52\xb8\x01\x00\x00\x00\xbf\x01\x00\x00\x00\xbe\x53\x54\x52\x57\xba\x0e\x00\x00\x00\x0f\x05\x5a\x5e\x5f\xb8\x41\x44\x44\x52\xff\xe0\x2e\x2e\x2e\x2e\x57\x4f\x4f\x44\x59\x2e\x2e\x2e\x2e\x0a";
	// unsigned char	payload[] = "\x57\x56\x52\xb8\x01\x00\x00\x00\xbf\x01\x00\x00\x00\x5a\x5e\x5f\xb8\x41\x44\x44\x52\xff\xe0\x2e\x2e\x2e\x2e\x57\x4f\x4f\x44\x59\x2e\x2e\x2e\x2e\x0a";
	//unsigned char	payload[] =  "\x57\x56\x52\xb8\x01\x00\x00\x00\xbf\x01\x00\x00\x00\xbe\x53\x54\x52\x57\xba\x0e\x00\x00\x00\x0f\x05\x5a\x5e\x5f\xb8\x41\x44\x44\x52\xff\xe0\x2e\x2e\x2e\x2e\x57\x4f\x4f\x44\x59\x2e\x2e\x2e\x2e\x0a";
	unsigned char	payload[] =  "\x57\x56\x52\xb8\x01\x00\x00\x00\xbf\x01\x00\x00\x00\x48\x8d\x35\x14\x00\x00\x00\xba\x0e\x00\x00\x00\x0f\x05\x5a\x5e\x5f\x48\x8d\x04\x25\x41\x44\x44\x52\xff\xe0\x2e\x2e\x2e\x2e\x57\x4f\x4f\x44\x59\x2e\x2e\x2e\x2e\x0a";
	size_t	payload_length = 55;
	void	*ptr;

	addr_to_str((unsigned char*)payload, "ADDR", old_entry); // Setting up the jump to the old entry
    (void)new_entry;
//	addr_to_str((unsigned char*)payload, "STRW", new_entry + payload_length - 14); // -14 = beggining of the string to print
	ptr = (void*)(file + shdr->sh_offset + shdr->sh_size + 8);
	memcpy(ptr, payload, payload_length);
}

uint64_t	get_payload_addr(Elf64_Phdr *phdr, Elf64_Shdr *shdr)
{
	uint64_t base_address;

	base_address = phdr->p_vaddr - phdr->p_offset;
	return (base_address + shdr->sh_offset + shdr->sh_size + 8);
}

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
    printf("Old payload: 0x%lx\nNew payload: 0x%lx\n", ehdr->e_entry, payload_addr);
	ehdr->e_entry = payload_addr;
	return (42);
}

void	iterate_over_program_headers(void *file)
{
	Elf64_Ehdr	*ehdr;
	Elf64_Phdr	*phdr;
	Elf64_Shdr	*shdr;
	uint64_t	old_entrypoint;
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
                printf("Choosen program header: %d\n", i);
				old_entrypoint = ehdr->e_entry;
				shdr = get_last_section(file, phdr->p_offset, phdr->p_offset + phdr->p_filesz);
				payload_addr = get_payload_addr(phdr, shdr);
				set_entrypoint(file, ehdr, payload_addr);
				insert_payload(file, payload_addr, shdr, old_entrypoint);
				prepare_section(shdr, PAYLOAD_SIZE);
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
