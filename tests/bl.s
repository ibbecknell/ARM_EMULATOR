	.global bl
	.func bl

bl:
	mov r0, #4
	add r0, r0, #6
	bl func
	add r0, r0,#1
	bx lr
func:
	mov r0,#0
	mov pc, lr
