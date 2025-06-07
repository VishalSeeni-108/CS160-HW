.text

.globl proc1
proc1:
	pushl	%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	%esp, %ebp
	subl	$4,%esp
	pushl	$0xa
	popl	%eax
	movl	%ebp, %esp
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	ret
.globl Main
Main:
	pushl	%ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	%esp, %ebp
	subl	$12,%esp
	pushl	$0x5
	popl	%eax
	pushl	%eax
	call	proc1
	addl $1*4,%esp
	pushl	%eax
	movl	$0,%eax
	pushl	%eax
	popl	%eax
	popl	%ebx
	movl %ebx,	(%ebp, %eax, 1)
	movl	0(%ebp),%eax
	pushl	%eax
	popl	%eax
	movl	%ebp, %esp
	popl	%edi
	popl	%esi
	popl	%ebx
	popl	%ebp
	ret
