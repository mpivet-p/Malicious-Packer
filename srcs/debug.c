#include "woody.h"
#include <stdio.h>

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
