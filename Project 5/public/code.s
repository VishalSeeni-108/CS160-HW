.text

.globl Main
Main:
	pushl	%ebx
	movl	$0,%eax
	pushl	%eax
	pushl	$0x5
	pushl	$0x7
	popl	%ebx
	popl	%eax
	addl	%ebx, %eax
	pushl	%eax
	popl	%ebx
	popl	%eax
	movl %ebx,	(%ebp, %eax, 1)
	movl	0(%ebp),%eax
	pushl	%eax
	popl	%eax
	popl	%ebx
	ret
