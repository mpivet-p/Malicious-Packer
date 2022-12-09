#pragma once

#include <stddef.h>
#include <elf.h>

#define PAYLOAD_SIZE 53
#define PAYLOAD_OPCODES "\x57\x56\x52\xb8\x01\x00\x00\x00\xbf\x01\x00\x00\x00\x48\x8d\x35\x11\x00\x00\x00\xba\x0e\x00\x00\x00\x0f\x05\x5a\x5e\x5f\xb8\x41\x44\x44\x52\xff\xe0\x2e\x2e\x2e\x2e\x57\x4f\x4f\x44\x59\x2e\x2e\x2e\x2e\x0a"

int	check_file(char *filepath);
int	get_file(const char *filepath, void **file, size_t *fsize);
int	woody(char *file_name, void *file, size_t fsize);
Elf64_Shdr	*get_section_header(void *file, char *name);
int		get_shstrtab_content(void *file, Elf64_Ehdr *ehdr, char **ptr);
