.global mov
.func mov

mov:
	mov r0, #5
	mov r1,#3
	cmp r0, r1
	moveq r0, r1
	bx lr
