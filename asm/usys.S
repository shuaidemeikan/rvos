#define SYS_gethid	1

.global gethid
gethid:
	li a7, SYS_gethid
	ecall
	ret
