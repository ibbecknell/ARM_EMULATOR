.global branch
.func branch

branch:
	mov r0, #3
	mov r1, #1
	b end
	add r3, r0,r1
	sub r1, #1
	mov r0,#0
	add r0,#4
end:
	bx lr
