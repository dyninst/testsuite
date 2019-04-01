	.arch armv8-a
	.file	"test6LS-aarch64.C"
	
	.extern divarw
	.zero	20
	.text
	.align	2
	.global	loadsnstores
	.type	loadsnstores, %function
loadsnstores:
.LFB0:
	sub	sp, sp, #16
.LCFI0:
	str	w0, [sp, 12]
	str	w1, [sp, 8]
	str	w2, [sp, 4]
	ldr	w0, [sp, 12] 
	ldr	w0, [sp, 8] 
	ldr	w0, [sp, 4] 
	
	ldr x0, =divarw 
	ldr x1, [x0]
	mov	x0, 0
	add	sp, sp, 16
.LCFI1:
	ret
.LFE0:
	.size	loadsnstores, .-loadsnstores
	.ident	"GCC: (GNU) 6.3.1 20161221 (Red Hat 6.3.1-1)"
	.section	.note.GNU-stack,"",@progbits
