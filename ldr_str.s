	.global ldr_str
	.func ldr_str

ldr_str:
	sub sp, sp, #4
	str r4,[sp]
	sub sp, sp, #4
	str r5, [sp]
	ldr r5,[sp]
	mov r5, #5
	mov r4, #4
	add sp,sp,#4
	ldr r4,[sp]
	add sp,sp,#4
	bx lr
