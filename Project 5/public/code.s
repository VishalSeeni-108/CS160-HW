.text

.globl Main
Main:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$260, %esp
	pushl	%ebx
	pushl	%esi
	pushl	%edi
.data

string_0:
.ascii	"test\0"
.text

	lea	string_0, %eax
	lea	-256(%ebp), %ebx # size 32901f0 256
	movb	(%eax), %cl
	movb	%cl, (%ebx)
	inc	%eax
	inc	%ebx
	movb	(%eax), %cl
	movb	%cl, (%ebx)
	inc	%eax
	inc	%ebx
	movb	(%eax), %cl
	movb	%cl, (%ebx)
	inc	%eax
	inc	%ebx
	movb	(%eax), %cl
	movb	%cl, (%ebx)
	inc	%eax
	inc	%ebx
	movb	(%eax), %cl
	movb	%cl, (%ebx)
	inc	%eax
	inc	%ebx
	pushl	$0x2
	popl	%edx
	xorl	%eax, %eax
	movb	-4(%ebp, %edx, 4), %al
	pushl	%eax
	pushl	$0x73
	popl	%ebx
	popl	%eax
	cmpl	%ebx, %eax
	je	true_2
	pushl	$0
	jmp	end_2
true_2:
	pushl	$1
end_2:
	popl	%eax
	cmpl	$1,%eax
	jne	else_1
	movl	$-260,%eax
	pushl	%eax
	pushl	$0x1
	popl	%ebx
	popl	%eax
	movl %ebx,	(%ebp, %eax, 1)
	jmp	end_1
else_1:
	movl	$-260,%eax
	pushl	%eax
	pushl	$0x0
	popl	%ebx
	popl	%eax
	movl %ebx,	(%ebp, %eax, 1)
end_1:
	movl	-260(%ebp),%eax
	pushl	%eax
	popl	%eax
	popl	%edi
	popl	%esi
	popl	%ebx
	movl	%ebp, %esp
	popl	%ebp
	ret
