.global find_str_a
.func find_str_a
/*r0= base address of string
r1 = base address of substring*/
find_str_a:
	sub sp,sp,#4
	str r4,[sp]
	sub sp,sp,#4
	str r5,[sp]
	sub sp,sp,#4
	str r6,[sp]
	sub sp,sp,#4
	str r7,[sp]
	mov r2,#0		//r2->i
	mov r3,#0		//r3->j
	mov r4,#-1		//r4->index
	ldrb r6,[r1,#0]		//check if substring is empty
	cmp r6,#0
	moveq r0,#-1
	beq end
outer_while:
	mov r7,r2
	ldrb r5,[r0,r2]		//get s[i]
	cmp r5,#0		//compare s[i] to null
	moveq r0,#-1
	beq end
	movne r4,r7
inner_while:
	ldrb r5,[r0,r2]
	ldrb r6,[r1,r3]
	cmp r6,r5		//compare sub[0] and s[i]
	bne if			//if sub[0] and s[i] are the same, go to if_1
	cmp r6,#0		//////new edit
	beq if			/////
	add r2,r2,#1
	add r3,r3,#1
	b inner_while
if:
	mov r2,r7
	cmp r6,#0
	moveq r0,r7
	beq end
	mov r7,r4
	mov r4,#1
end_outer_while:
	add r2,r2,#1
	mov r3,#0
	b outer_while
end:
	ldr r7,[sp]
	add sp,sp,#4
	ldr r6,[sp]
	add sp,sp,#4
	ldr r5,[sp]
	add sp,sp,#4
	ldr r4,[sp]
	add sp,sp,#4
	bx lr
