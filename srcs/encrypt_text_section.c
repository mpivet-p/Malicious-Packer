#include "woody.h"
#include <stdio.h>
#include <stdlib.h>

void	encrypt_text_section(void *file)
{
	Elf64_Shdr *text_sect;

	text_sect = get_section_header(file, ".text");
	if (!text_sect)
	{
		fprintf(stderr, "woody: no .text section found");
		exit(1);
	}
	printf(".text section is located at address: 0x%lx\n", text_sect->sh_addr);
	file += text_sect->sh_offset;
	for (ssize_t i = text_sect->sh_size - 1; i >= 0; i--)
	{
		*(char*)(file + i) = *(char*)(file + i) + 1;
	}
}
