#include "woody.h"
#include <elf.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Elf64_Shdr	*get_last_section(void *file, uint64_t min, uint64_t max)
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
	return (last_shdr);
}

static void	increase_segment_size(Elf64_Phdr *phdr, size_t sizediff)
{
	phdr->p_filesz += sizediff;
	phdr->p_memsz += sizediff;
}

static void	copy_payload(void *file, Elf64_Shdr *shdr)
{
	unsigned char	payload[] =  PAYLOAD_OPCODES;
	void	*ptr;

	ptr = (void*)(file + shdr->sh_offset + shdr->sh_size);
	ft_memcpy(ptr, payload, PAYLOAD_SIZE);
}

static uint64_t	get_payload_addr(Elf64_Phdr *phdr, Elf64_Shdr *shdr)
{
	uint64_t base_address;

	base_address = phdr->p_vaddr - phdr->p_offset;
	return (base_address + shdr->sh_offset + shdr->sh_size);
}

static void	config_payload(void *file, Elf64_Shdr *shdr, uint32_t jmp_addr, uint32_t key)
{
	Elf64_Shdr	*text_shdr;
	void		*ptr;

	text_shdr = get_section_header(file, ".text");
	//Setting up the jump
	ptr = (void*)((file + shdr->sh_offset + shdr->sh_size) + PAYLOAD_SIZE - 22);
	*(uint32_t*)(ptr) = jmp_addr;

	//.text section size
	ptr = (void*)((file + shdr->sh_offset + shdr->sh_size) + 69);
	*(uint32_t*)(ptr) = text_shdr->sh_size;

	//.text relative addr
	ptr = (void*)((file + shdr->sh_offset + shdr->sh_size) + 37);
	*(uint32_t*)(ptr) = (uint32_t)(text_shdr->sh_addr - (shdr->sh_addr + shdr->sh_size));

	//Setting up the key
	ptr = (void*)((file + shdr->sh_offset + shdr->sh_size + PAYLOAD_SIZE - 3));
	*(uint32_t*)(ptr) = key;
}

static void	config_mprotect(void *file, Elf64_Phdr *phdr, Elf64_Shdr *shdr)
{
	Elf64_Shdr	*text_shdr;
	void		*ptr;

	text_shdr = get_section_header(file, ".text");

	//page address
	ptr = (void*)((file + shdr->sh_offset + shdr->sh_size) + 52);
	*(uint32_t*)(ptr) = (uint32_t)(phdr->p_vaddr - text_shdr->sh_addr);

	//page size
	ptr = (void*)((file + shdr->sh_offset + shdr->sh_size) + 57);
	*(uint32_t*)(ptr) = (uint32_t)phdr->p_memsz;
}

void	inject(void *file, Elf64_Ehdr *ehdr, Elf64_Phdr *phdr, uint32_t key)
{
	Elf64_Shdr	*shdr;
	uint32_t	jmp_addr;
	uint64_t	payload_addr;

	shdr = get_last_section(file, phdr->p_offset, phdr->p_offset + phdr->p_filesz);
	payload_addr = get_payload_addr(phdr, shdr);
    jmp_addr = (uint32_t)(ehdr->e_entry - (payload_addr + PAYLOAD_SIZE - 18));

	copy_payload(file, shdr);
	config_payload(file, shdr, jmp_addr, key);
	config_mprotect(file, phdr, shdr);
    ehdr->e_entry = payload_addr;
	increase_segment_size(phdr, PAYLOAD_SIZE);
}
