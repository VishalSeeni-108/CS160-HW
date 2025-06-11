.text

.globl Main
Main:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	subl	$1024, %esp
.data
String0:
    .asciz "test"
.text
    leal String0, %eax
    movzbl 0(%eax), %ebx
    movl   %ebx, -4(%ebp)
    movzbl 1(%eax), %ebx
    movl   %ebx, -8(%ebp)
    movzbl 2(%eax), %ebx
    movl   %ebx, -12(%ebp)
    movzbl 3(%eax), %ebx
    movl   %ebx, -16(%ebp)
	movl	$-1024,%eax
	pushl	%eax
   pushl $255
	popl	%ebx
	popl	%eax
	movl %ebx,	(%ebp, %eax, 1)
	movl	-1024(%ebp),%eax
	pushl	%eax
	popl	%eax
	popl	%edi
	popl	%esi
	popl	%ebx
	movl	%ebp, %esp
	popl	%ebp
	ret
