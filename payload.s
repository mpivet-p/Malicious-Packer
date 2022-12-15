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
	add r9, "BGIN"

mem_protect:
	mov rax, 10			; mprotect syscall code
	mov rdi, r9 		; addr
	add rdi, "SGMT"
	mov rsi, "SSIZ"		; size
	mov rdx, 7			; read, write, exec
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
key: db "xKEY"
