    .text
	.globl	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq    $24,  %rsp	
    
    movq	$5, -8(%rbp)
	movq	$10, -16(%rbp)
	movq	$15, -24(%rbp)
	movq	$0, %rax	

    addq    $24, %rsp
	popq	%rbp
	ret
.LFE0:
	.size	main, .-main
	.section	.note.GNU-stack,"",@progbits
