#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>

enum reg { REG_AX, REG_BX, REG_CX, REG_DX, REG_PC, REG_IR, REG_LIMIT};

struct cpu {
    int32_t int_regs[REG_LIMIT];
    FILE *instmem;
};
struct cpu *cpu_new(void);
int32_t cpu_reg(enum reg reg);
int32_t cpu_load_instfile(struct cpu *self, const char *filename);
int32_t cpu_next_cycle(struct cpu *self);

#endif
