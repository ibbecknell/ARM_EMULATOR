	.global ldr_str
	.func ldr_str

ldr_str:
	mov r0, #4
	sub sp, sp, #4
	str r0,[sp]
	sub sp, sp, #4
	mov r5, #24
	str r5,	[sp]	
	ldr r5,[sp]
	add sp,sp,#4
	ldr r0,[sp]
	add sp,sp,#4
	bx lr
