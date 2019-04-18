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
	ldr x0, =divarw 
	ldr x1, =divarw 
	ldr x2, =divarw 
	ldr x3, =divarw 
	ldr x4, =divarw 
	ldr x5, [x1] 
	ldr x6, [x2] 
	ldr x7, [x3]
	ldr x8, [x4]
	
	/*No. 9*/		
	ldr x8, [x2, 4]	
	ldr x8, [x3, 12]	
	/*	
	ldr x5, [x1, x2]
	*/
	add	sp, sp, 16
.LCFI1:
	ret
.LFE0:
	.size	loadsnstores, .-loadsnstores
	.ident	"GCC: (GNU) 6.3.1 20161221 (Red Hat 6.3.1-1)"
	.section	.note.GNU-stack,"",@progbits
