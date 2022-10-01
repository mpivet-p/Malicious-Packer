#include <elf.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

//static int		get_file(const char *filepath, void **file_content, size_t *file_size)
//{
//	int			fd;
//
//	if ((fd = open(filepath, O_RDONLY)) < 0)
//	{
//		fprintf(stderr, "woody: Error opening the file.\n");
//		return (1);
//	}
//	*file_size = lseek(fd, 0, SEEK_END);
//	lseek(fd, 0, SEEK_SET);
//	*file_content = mmap(NULL, *file_size, PROT_READ, MAP_PRIVATE, fd, 0);
//	close(fd);
//	return (*file_content != MAP_FAILED);
//}

void	handle_read_return(int ret)
{
	if (ret <= 0)
	{
		perror("woody: ");
		exit(1);
	}
}

uint64_t	get_image_base(int fd)
{
	Elf64_Ehdr	ehdr;
	Elf64_Phdr	phdr;

	lseek(fd, 0, SEEK_SET);
	handle_read_return(read(fd, &ehdr, sizeof(ehdr)));
	
	lseek(fd, ehdr.e_phoff, SEEK_SET);
	for (int i = 0; i < ehdr.e_phnum; i++)
	{
		if (phdr.p_type == PT_LOAD)
		{
			printf("addr: 0x%lx\n", phdr.p_vaddr);
			return (phdr.p_vaddr);
		}
		handle_read_return(read(fd, &phdr, sizeof(phdr)));
	}
	return (0);
}

int		get_shstrtab_content(int fd, Elf64_Ehdr *ehdr, char **ptr)
{
	Elf64_Shdr	shdr;
	int			ret;

	lseek(fd, ehdr->e_shoff + (sizeof(shdr) * ehdr->e_shstrndx), SEEK_SET);
	handle_read_return(read(fd, &shdr, sizeof(shdr)));
	lseek(fd, shdr.sh_offset, SEEK_SET);
	if (!(*ptr = (char*)malloc((shdr.sh_size + 1) * sizeof(char))))
		exit(2);
	if ((ret = read(fd, *ptr, shdr.sh_size)) < (ssize_t)(shdr.sh_size))
	{
		free(*ptr);
		return (1);
	}
	(*ptr)[ret] = 0;
	return (0);
}
int		is_pt_load(int fd, Elf64_Ehdr *ehdr, uint64_t offset)
{
	Elf64_Phdr	phdr;
	int			f_offset;

	f_offset = lseek(fd, 0, SEEK_CUR);
	lseek(fd, ehdr->e_phoff, SEEK_SET);
	for (int i = 0; i < ehdr->e_phnum; i++)
	{
		handle_read_return(read(fd, &phdr, sizeof(phdr)));
		if (offset >= phdr.p_offset
			&& offset < (phdr.p_offset + phdr.p_filesz))
		{
			lseek(fd, f_offset, SEEK_SET);
			return (1);
		}
	}
	lseek(fd, f_offset, SEEK_SET);
	return (0);
}

void	iterate_over_section_headers(int fd)
{
	Elf64_Ehdr	ehdr;
	Elf64_Shdr	shdr;
	char		*shstrtab_content;

	lseek(fd, 0, SEEK_SET);
	handle_read_return(read(fd, &ehdr, sizeof(ehdr)));
	get_shstrtab_content(fd, &ehdr, &shstrtab_content);

	lseek(fd, ehdr.e_shoff, SEEK_SET);
	for (int i = 0; i < ehdr.e_shnum; i++)
	{
		handle_read_return(read(fd, &shdr, sizeof(shdr)));
		printf("addr: %lx, offset: %lx, size: %ld, sh_name: %s", shdr.sh_addr\
		, shdr.sh_offset, shdr.sh_size, shstrtab_content + shdr.sh_name);
		if (shdr.sh_flags & SHF_EXECINSTR)
			printf(" EXEC");
		if (is_pt_load(fd, &ehdr, shdr.sh_offset + shdr.sh_size) == 1)
			printf(" PT_LOAD");
		printf("\n");
	}
	free(shstrtab_content);
}

int	main(int argc, char **argv)
{
	int	fd;

	if (argc > 1)
	{
		if ((fd = open(argv[1], O_RDONLY)) >= 0)
		{
			get_image_base(fd);
			iterate_over_section_headers(fd);
		}
		else
		{
			perror("woody: ");
			return (1);
		}
	}
	return (0);
}
