	.arch armv8-a
	.file	"test6LS-aarch64.C"
	.text
	.align	2
	.global	loadsnstores
	.type	loadsnstores, %function
loadsnstores:
.LFB0:
	sub	sp, sp, #8
.LCFI0:
	ldr	w0, [sp, 44]
	mov	x0, 0
	add	sp, sp, 8
.LCFI1:
	ret
.LFE0:
	.size	loadsnstores, .-loadsnstores
	.ident	"GCC: (GNU) 6.3.1 20161221 (Red Hat 6.3.1-1)"
	.section	.note.GNU-stack,"",@progbits
