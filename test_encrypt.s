global _start

_start:
	mov rcx, 7

loop:
	dec rcx
	mov	rax, rcx
	mov rdi, 4
	div rdi
	mov al, byte [rel key + rdx]
	mov	bl, byte [rel msg + rcx]
	xor al, bl
	test rcx, rcx
	jnz loop

msg: db 0xeb, 0x18, 0x7d, 0x05, 0xd1, 0x00, 0xd3
key: db 0xff, 0x00, 0xa2, 0x1c
