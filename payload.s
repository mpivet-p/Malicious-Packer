global _start

_start:
	push rdi
	push rsi
	push rdx
	mov	rax, 1	;write syscall
	mov	rdi, 1	;STDOUT_FILENO
;	mov rsi, 0x57525453
;	mov rdx, msglen
	push 0x57
    mov rsi, rsp
	mov rdx, 1
	syscall
    pop rdx
	pop rdx
	pop rsi
	pop rdi
	lea rax, 0x52444441
	jmp rax

msg: db "....WOODY....", 10
msglen: equ $ - msg
