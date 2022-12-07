global _start

_start:
	push rdi
	push rsi
	push rdx
	mov	rax, 1	;write syscall
	mov	rdi, 1	;STDOUT_FILENO
    lea rsi, [rel msg]
	mov rdx, msg_end - msg
	syscall
	pop rdx
	pop rsi
	pop rdi
	lea rax, [0x52444441]
	jmp rax

msg: db "....WOODY....", 10
msg_end: db 0x0
