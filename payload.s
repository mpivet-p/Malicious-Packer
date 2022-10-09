global _start

section .text

	msg: db "Hello World!", 10

	_end:
		mov	rax, 60
		mov	rdi, 42
		syscall


	_start:
		mov	rax, 1	;write syscall
		mov	rdi, 1	;STDOUT_FILENO
		mov	rsi, 0x401020
		mov	rdx, 13
		syscall
		mov rax, 0x401020
		jmp rax
