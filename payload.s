global _start

_start:
	push rdi
	push rsi
	push rdx

print_woody:
	mov	rax, 1
	mov	rdi, 1
    lea rsi, [rel msg]
	mov rdx, msg_end - msg
	syscall

	mov r8, "SIZE"
	lea r9, [rel _start]
	sub r9, "BGIN"
	mov r10, "YYEK"

mem_protect:
	mov rax, 10
	mov rdi, r9
	mov rsi, r8
	mov rdx, 7
	syscall

decrypt_loop:
	mov bl, byte [r9]
	dec bl
	mov byte [r9], bl
	inc r9
	dec r8
	test r8, r8
	jnz decrypt_loop

end:
	pop rdx
	pop rsi
	pop rdi
    jmp 0xffffffff

msg: db "....WOODY....", 10
msg_end: db 0x0
