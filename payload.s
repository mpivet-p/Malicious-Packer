global _start

_start:
	push rdi
	push rsi
	push rdx
	mov	rax, 1	;write syscall
	mov	rdi, 1	;STDOUT_FILENO
	mov rsi, 0x57525453
	mov rdx, msglen
	syscall
	pop rdx
	pop rsi
	pop rdi
	mov rax, 0x52444441
	jmp rax

msg: db "....WOODY....", 10
msglen: equ $ - msg
