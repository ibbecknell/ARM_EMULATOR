.global find_max_a
.func find_max_a

find_max_a:
	mov r2, r0
	mov r3, #0
	ldr r0, [r2]
loop:
	cmp r3, r1
	beq loopend
	ldr r12,[r2]
	cmp r12, r0
	movgt r0, r12
	add r3,r3,#1
	add r2,r2,#4
	b loop
loopend:
	bx lr      
