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

	mov rdx, "YYEK"
	mov rsi, "END"
	mov	rcx, "BGIN"
decrypt_loop:
	mov bl, byte [rel rsi]
	dec bl
	mov byte [rel rsi], bl
	inc rcx
	test rcx, rsi
	jle decrypt_loop

end:
	pop rdx
	pop rsi
	pop rdi
    jmp 0xffffffff

msg: db "....WOODY....", 10
msg_end: db 0x0
