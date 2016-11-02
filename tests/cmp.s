.global cmp
.func cmp

cmp:
	cmp r0, #5
	mov r1, #1
	cmp r0, r1
	bx lr
