.global fib_rec_a
.func fib_rec_a

fib_rec_a:
	sub sp, sp, #4
	str lr,[sp]
	sub sp, sp, #4
	str r5,[sp]
	sub sp, sp,#4
	str r4, [sp]
	mov r4, r0
	cmp r0, #0
	beq fib_exit
	cmp r0,#1
	beq fib_exit
fib_rec:
	sub r0, r4, #1
	bl fib_rec_a
	mov r5, r0
	sub r0, r4, #2
	bl fib_rec_a
	add r0, r5, r0
fib_exit:
	ldr r4,[sp]
	add sp, sp,#4
	ldr r5,[sp]
	add sp, sp,#4
	ldr lr,[sp]
	add sp, sp, #4
	bx lr
	
	
	
