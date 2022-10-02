#pragma once

#include <stddef.h>
#define PAYLOAD_SIZE 80

int	check_file(char *filepath);
int	get_file(const char *filepath, void **file, size_t *fsize);
int	woody(char *file_name, void *file, size_t fsize);