include ./assist_file/common.mk

SYSCALL = y

ifeq (${SYSCALL}, y)
CFLAGS += -D CONFIG_SYSCALL
endif
SRCS_ASM= $(wildcard asm/*.S)
SRCS_C = $(wildcard src/*.c)
SRCS_INC += $(wildcard src/*.h)

OBJS = $(SRCS_ASM:.S=.o)
OBJS += $(SRCS_C:.c=.o)

.DEFAULT_GOAL := all
all: os.elf

# start.o must be the first in dependency!
os.elf: ${OBJS}
	${CC} ${CFLAGS} -T ./assist_file/os.ld -o build/os.elf build/start.o $(filter-out build/start.o, $(wildcard build/*.o)) 
	${OBJCOPY} -O binary build/os.elf build/os.bin

%.o : %.c
	${CC} ${CFLAGS} -Iinclude -c -o $@ $<
	mv ./src/*.o ./build

%.o : %.S
	${CC} ${CFLAGS} -Iinclude -c -o $@ $<
	mv ./asm/*.o ./build

run: all
	@${QEMU} -M ? | grep virt >/dev/null || exit
	@echo "Press Ctrl-A and then X to exit QEMU"
	@echo "------------------------------------"
	@${QEMU} ${QFLAGS} -kernel build/os.elf

.PHONY : debug
debug: all
	@echo "Press Ctrl-C and then input 'quit' to exit GDB and QEMU"
	@echo "-------------------------------------------------------"
	@${QEMU} ${QFLAGS} -kernel build/os.elf -s -S &
	@${GDB} build/os.elf -q -x ../gdbinit

vscodedebug: all
	@echo "Press Ctrl-C and then input 'quit' to exit GDB and QEMU"
	@echo "-------------------------------------------------------"
	@${QEMU} ${QFLAGS} -kernel build/os.elf -s -S &

.PHONY : code
code: all
	@${OBJDUMP} -S build/os.elf | less

.PHONY : clean
clean:
	rm -rf build/*.o build/*.bin build/*.elf

