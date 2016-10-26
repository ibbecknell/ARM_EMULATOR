.global sum_array_a
.func sum_array_a

sum_array_a:
	mov r2, r0
	mov r3, #0
	mov r0, #0
loop:
	cmp r3, r1
	beq loopend
	ldr r12,[r2]
	add r0,r0,r12
	add r3,r3,#1
	add r2,r2,#4
	b loop
loopend:
	bx lr      
