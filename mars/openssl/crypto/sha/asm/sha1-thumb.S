.text
.code	16

.global	sha1_block_data_order
.type	sha1_block_data_order,%function

.align	2
sha1_block_data_order:
	push	{r4-r7}
	mov	r3,r8
	mov	r4,r9
	mov	r5,r10
	mov	r6,r11
	mov	r7,r12
	push	{r3-r7,lr}
	lsl	r2,#6
	mov	r9,r0			@ save context
	mov	r10,r1			@ save inp
	mov	r11,r2			@ save len
	add	r11,r10		@ r11 to point at inp end

.Lloop:
	mov	r12,sp
	mov	r2,sp
	sub	r2,#16*4		@ [3]
.LXload:
	ldrb	r3,[r1,#0]		@ r1 is r1 and holds inp
	ldrb	r4,[r1,#1]
	ldrb	r5,[r1,#2]
	ldrb	r6,[r1,#3]
	lsl	r3,#24
	lsl	r4,#16
	lsl	r5,#8
	orr	r3,r4
	orr	r3,r5
	orr	r3,r6
	add	r1,#4
	push	{r3}
	cmp	sp,r2
	bne	.LXload			@ [+14*16]

	mov	r10,r1		@ update r10
	sub	r2,#32*4
	sub	r2,#32*4
	mov	r7,#31			@ [+4]
.LXupdate:
	ldr	r3,[sp,#15*4]
	ldr	r4,[sp,#13*4]
	ldr	r5,[sp,#7*4]
	ldr	r6,[sp,#2*4]
	eor	r3,r4
	eor	r3,r5
	eor	r3,r6
	ror	r3,r7
	push	{r3}
	cmp	sp,r2
	bne	.LXupdate		@ [+(11+1)*64]

	ldmia	r0!,{r3,r4,r5,r6,r7}	@ r0 is r0 and holds ctx
	mov	r0,r12

	ldr	r2,.LK_00_19
	mov	r1,r0
	sub	r1,#20*4
	mov	r12,r1
	mov	r8,r2			@ [+7+4]
.L_00_19:
	bl	.Lcommon
	mov	r1,r5
	eor	r1,r6
	and	r1,r4
	eor	r1,r6			@ F_00_19(B,C,D)
	bl	.Lrotate
	cmp	r12,r0
	bne	.L_00_19		@ [+(2+9+4+2+8+2)*20]

	ldr	r2,.LK_20_39
	mov	r1,r0
	sub	r1,#20*4
	mov	r12,r1
	mov	r8,r2			@ [+5]
.L_20_39_or_60_79:
	bl	.Lcommon
	mov	r1,r4
	eor	r1,r5
	eor	r1,r6			@ F_20_39(B,C,D)
	bl	.Lrotate
	cmp	r12,r0
	bne	.L_20_39_or_60_79	@ [+(2+9+3+2+8+2)*20*2]
	cmp	sp,r0
	beq	.Ldone			@ [+2]

	ldr	r2,.LK_40_59
	mov	r1,r0
	sub	r1,#20*4
	mov	r12,r1
	mov	r8,r2			@ [+5]
.L_40_59:
	bl	.Lcommon
	mov	r1,r4
	and	r1,r5
	mov	r7,r4
	orr	r7,r5
	and	r7,r6
	orr	r1,r7			@ F_40_59(B,C,D)
	bl	.Lrotate
	cmp	r12,r0
	bne	.L_40_59		@ [+(2+9+6+2+8+2)*20]

	ldr	r2,.LK_60_79
	mov	r12,sp
	mov	r8,r2
	b	.L_20_39_or_60_79	@ [+4]
.Ldone:
	mov	r0,r9
	ldr	r1,[r0,#0]
	ldr	r2,[r0,#4]
	add	r3,r1
	ldr	r1,[r0,#8]
	add	r4,r2
	ldr	r2,[r0,#12]
	add	r5,r1
	ldr	r1,[r0,#16]
	add	r6,r2
	add	r7,r1
	stmia	r0!,{r3,r4,r5,r6,r7}	@ [+20]

	add	sp,#80*4		@ deallocate stack frame
	mov	r0,r9		@ restore ctx
	mov	r1,r10		@ restore inp
	cmp	r1,r11
	beq	.Lexit
	b	.Lloop			@ [+6] total 3212 cycles
.Lexit:
	pop	{r2-r7}
	mov	r8,r2
	mov	r9,r3
	mov	r10,r4
	mov	r11,r5
	mov	r12,r6
	mov	lr,r7
	pop	{r4-r7}
	bx	lr
.align	2
.Lcommon:
	sub	r0,#4
	ldr	r1,[r0]
	add	r7,r8			@ E+=K_xx_xx
	lsl	r2,r3,#5
	add	r2,r7
	lsr	r7,r3,#27
	add	r2,r7			@ E+=ROR(A,27)
	add	r2,r1			@ E+=X[i]
	mov	pc,lr
.Lrotate:
	mov	r7,r6			@ E=D
	mov	r6,r5			@ D=C
	lsl	r5,r4,#30
	lsr	r4,r4,#2
	orr	r5,r4			@ C=ROR(B,2)
	mov	r4,r3			@ B=A
	add	r3,r2,r1		@ A=E+F_xx_xx(B,C,D)
	mov	pc,lr
.align	2
.LK_00_19:	.word	0x5a827999
.LK_20_39:	.word	0x6ed9eba1
.LK_40_59:	.word	0x8f1bbcdc
.LK_60_79:	.word	0xca62c1d6
.size	sha1_block_data_order,.-sha1_block_data_order
.asciz	"SHA1 block transform for Thumb, CRYPTOGAMS by <appro@openssl.org>"
