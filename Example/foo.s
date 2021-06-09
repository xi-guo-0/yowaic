	.data
.L0:
	.string "%d\n"
.L1:
	.string "*b = %d\n"
.L3:
	.string "%c "
.L4:
	.string "\n"
.L5:
	.string "%c "
.L6:
	.string "\n"
	.text
	.global fibonacci
fibonacci:
	pushq %rbp
	movq %rsp, %rbp
	push %rdi
	subq $40, %rsp
	movl $0, %eax
	movl %eax, -16(%rbp)
	movl $1, %eax
	movl %eax, -24(%rbp)
	movl $1, %eax
	movl %eax, -32(%rbp)
	movl $0, %eax
	movl %eax, -40(%rbp)
	.L7:
	movl -40(%rbp), %eax
	pushq %rax
	movl -8(%rbp), %eax
	popq %rcx
	cmpq %rax, %rcx
	setl %al
	movzb %al, %eax
	test %rax, %rax
	je .L8
	push %rsi
	leaq .L0(%rip), %rax
	pushq %rax
	movl -16(%rbp), %eax
	pushq %rax
	pop %rsi
	pop %rdi
	movq $0, %rax
	call printf@plt
	pop %rsi
	movl -16(%rbp), %eax
	push %rax
	movl -24(%rbp), %eax
	popq %rcx
	add %rcx, %rax
	movl %eax, -32(%rbp)
	movl -24(%rbp), %eax
	movl %eax, -16(%rbp)
	movl -32(%rbp), %eax
	movl %eax, -24(%rbp)
	movl -40(%rbp), %eax
	push %rax
	movl $1, %eax
	popq %rcx
	add %rcx, %rax
	movl %eax, -40(%rbp)
	jmp .L7
	.L8:
	movl $0, %eax
	leave
	ret
leave
	ret
.text
	.global test_array
test_array:
	pushq %rbp
	movq %rsp, %rbp
	subq $56, %rsp
	movl $20, %eax
	movl %eax, -16(%rbp)
	movl $30, %eax
	movl %eax, -12(%rbp)
	movl $40, %eax
	movl %eax, -8(%rbp)
	leaq -16(%rbp), %rax
	pushq %rax
	movl $1, %eax
	imulq $4, %rax
	movq %rax, %rbx
	popq %rax
	addq %rbx, %rax
	movq %rax, -24(%rbp)
	push %rsi
	leaq .L1(%rip), %rax
	pushq %rax
	movq -24(%rbp), %rax
	movl $0, %ebx
	mov (%rax), %ebx
	movq %rbx, %rax
	pushq %rax
	pop %rsi
	pop %rdi
	movq $0, %rax
	call printf@plt
	pop %rsi
	movb $116, -48(%rbp)
	movb $104, -47(%rbp)
	movb $105, -46(%rbp)
	movb $115, -45(%rbp)
	movb $32, -44(%rbp)
	movb $105, -43(%rbp)
	movb $115, -42(%rbp)
	movb $32, -41(%rbp)
	movb $97, -40(%rbp)
	movb $32, -39(%rbp)
	movb $115, -38(%rbp)
	movb $105, -37(%rbp)
	movb $110, -36(%rbp)
	movb $103, -35(%rbp)
	movb $108, -34(%rbp)
	movb $101, -33(%rbp)
	movb $32, -32(%rbp)
	movb $115, -31(%rbp)
	movb $116, -30(%rbp)
	movb $114, -29(%rbp)
	movb $105, -28(%rbp)
	movb $110, -27(%rbp)
	movb $103, -26(%rbp)
	movb $0, -25(%rbp)
	leaq -48(%rbp), %rax
	movq %rax, -56(%rbp)
	.L9:
	movq -56(%rbp), %rax
	movl $0, %ebx
	mov (%rax), %bl
	movq %rbx, %rax
	test %rax, %rax
	je .L10
	push %rsi
	leaq .L3(%rip), %rax
	pushq %rax
	movq -56(%rbp), %rax
	movl $0, %ebx
	mov (%rax), %bl
	movq %rbx, %rax
	pushq %rax
	pop %rsi
	pop %rdi
	movq $0, %rax
	call printf@plt
	pop %rsi
	movq -56(%rbp), %rax
	pushq %rax
	movl $1, %eax
	movq %rax, %rbx
	popq %rax
	addq %rbx, %rax
	movq %rax, -56(%rbp)
	jmp .L9
	.L10:
	leaq .L4(%rip), %rax
	pushq %rax
	pop %rdi
	movq $0, %rax
	call printf@plt
	leaq -48(%rbp), %rax
	movq %rax, -56(%rbp)
	.L11:
	movq -56(%rbp), %rax
	movl $0, %ebx
	mov (%rax), %bl
	movq %rbx, %rax
	test %rax, %rax
	je .L12
	push %rsi
	leaq .L5(%rip), %rax
	pushq %rax
	movq -56(%rbp), %rax
	movl $0, %ebx
	mov (%rax), %bl
	movq %rbx, %rax
	pushq %rax
	pop %rsi
	pop %rdi
	movq $0, %rax
	call printf@plt
	pop %rsi
	movq -56(%rbp), %rax
	pushq %rax
	movl $2, %eax
	movq %rax, %rbx
	popq %rax
	addq %rbx, %rax
	movq %rax, -56(%rbp)
	jmp .L11
	.L12:
	leaq .L6(%rip), %rax
	pushq %rax
	pop %rdi
	movq $0, %rax
	call printf@plt
	movl $0, %eax
	leave
	ret
leave
	ret
.text
	.global main
main:
	pushq %rbp
	movq %rsp, %rbp
	movl $10, %eax
	push %rax
	movl $3, %eax
	popq %rcx
	add %rcx, %rax
	pushq %rax
	pop %rdi
	movq $0, %rax
	call fibonacci
	movq $0, %rax
	call test_array
	movl $0, %eax
	leave
	ret
leave
	ret
