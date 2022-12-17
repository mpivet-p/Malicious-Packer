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

	lea r9, [rel _start]
	add r9, "BGIN"
mem_protect:
	mov rax, 10			; mprotect syscall code
	mov rdi, r9 		; addr
	add rdi, "SGMT"
	mov rsi, "SSIZ"		; size
	mov rdx, 7			; read, write, exec
	syscall

	mov rcx, "SIZE"
	lea	r10, [rel key]
decrypt_loop:
	dec rcx
	mov bl, byte [r9 + rcx]

	xor rdx, rdx
	mov rax, rcx
	mov rdi, 4			; 4 = key length
	div rdi
	mov al, byte [r10 + rdx]
	xor bl, al

	mov byte [r9 + rcx], bl
	test rcx, rcx
	jnz decrypt_loop

end:
	pop rdx
	pop rsi
	pop rdi
    jmp 0xffffffff

msg: db "....WOODY....", 10
msg_end: db 0x0
key: db "xKEY"
