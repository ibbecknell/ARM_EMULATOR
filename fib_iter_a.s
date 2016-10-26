.global fib_iter_a
.func fib_iter_a

fib_iter_a:
	mov r1, #2
	sub r2, r1,#2
	sub r3, r1,#1
	mov r4, #0
	cmp r0,#0
	beq endloop
	cmp r0,#1
	beq endloop
loop:
	cmp r1,r0
	movgt r0,r4 
	bgt endloop
	add r4, r2,r3
	mov r2,r3
	mov r3,r4
	add r1,r1,#1
	b loop
endloop:
	bx lr
