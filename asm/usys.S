#define SYS_gethid	1
#define SYS_printf	2

.global gethid
gethid:
	li a7, SYS_gethid
	ecall
	ret

.global sysprintf
sysprintf:
	li a7, SYS_printf
	ecall
	ret	