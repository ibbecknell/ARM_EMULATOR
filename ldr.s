	.global ldr
	.func ldr

ldr:	
	mov r2, r0
	ldr r3, [r2]
	add r2,r2,#4
	ldr r3, [r2]
	mov r0, r3
	bx lr
