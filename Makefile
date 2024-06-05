OBJECTS  = main.o irq.o bootstrap.o
CFLAGS   = -march=rv32i -mabi=ilp32 
LDFLAGS  = -nostartfiles -Wl,--no-relax

RISCV_GCC    = riscv32-unknown-elf-gcc
RISCV_CFLAGS = irq.c bootstrap.S -march=rv32ima -mabi=ilp32 -misa-spec=2.2 -Wall -Wno-unused-variable -O3
RISCV_LFLAGS = -nostartfiles -Wl,--no-relax
RISCV_VP     = riscv-vp
RISCV_VPOPT  = --intercept-syscalls --use-dmi --tlm-global-quantum 100000

all: space_invaders

test: sim


clean:
	rm -f *.bak *~
	rm -f *.o
	rm -f ${FRAMES}
	rm -f ${SFRAMES}

space_invaders: main.c irq.c irq.h bootstrap.S
	${RISCV_GCC} main.c ${RISCV_CFLAGS} ${RISCV_LFLAGS} -lm -o space_invaders


sim: space_invaders
	${RISCV_VP} ${RISCV_VPOPT} space_invaders