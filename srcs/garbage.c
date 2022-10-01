Elf64_Phdr	get_segment_header(int fd, Elf64_Ehdr *ehdr, uint64_t offset)
{
	Elf64_Phdr	phdr;
	int			f_offset;

	f_offset = lseek(fd, 0, SEEK_CUR);
	lseek(fd, ehdr->e_phoff, SEEK_SET);
	for (int i = 0; i < ehdr->e_phnum; i++)
	{
		handle_read_return(read(fd, &phdr, sizeof(phdr)));
		if (offset >= phdr.p_offset && offset < (phdr.p_offset + phdr.p_filesz))
		{
			printf("           [%d] -> type: %d, flags: %d", i, phdr.p_type, phdr.p_flags);
			break ;
		}
	}
	lseek(fd, f_offset, SEEK_SET);
	return (phdr);
}
		//printf("WOODY: file opened!\n");
		//file_header = file_content;
		//printf("Entry Point: 0x%lx\n", file_header->e_entry);
		//munmap(file_content, file_size);
//	Elf64_Ehdr	*file_header;
//	void		*file_content = NULL;
//	size_t			file_size = 0;


