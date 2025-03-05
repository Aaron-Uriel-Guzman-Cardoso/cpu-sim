#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>

enum reg { REG_AX, REG_BX, REG_CX, REG_DX, REG_PC, REG_IR, REG_LIMIT};
enum cpu_state { 
    CPU_READY,      /* Lista para ejecutar nueva instrucción */
    CPU_HALT,       /* Se ejecutó END o final de archivo */
    CPU_IR_ERROR    /* Instrucciones ilegales (¿será esto necesario?) */
};
enum consts {
    INSTS_MAX = 128,
    INSTS_STR_MAX = 32
};

struct cpu {
    int32_t int_regs[REG_LIMIT];
    enum cpu_state state;
    /*
     * La memoria para las instrucciones será un arreglo de 128 x 32,
     * esperando que no nos pasemos de 128 instrucciones 
     */
    char instmem[INSTS_MAX][INSTS_STR_MAX];
};
struct cpu *cpu_new(void);
int32_t cpu_reset(struct cpu *self);
int32_t cpu_load_insts_from_file(struct cpu *self, const char *filename);
int32_t cpu_load_insts_from_str(struct cpu *self, char *str);
int32_t cpu_next_cycle(struct cpu *self);

#endif
