#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>
#include <insts.h>


enum cpu_state { 
    CPU_READY,      /* Lista para ejecutar nueva instrucción */
    CPU_HALT,       /* Se ejecutó END o final de archivo */
    CPU_IR_ERROR    /* Instrucciones ilegales (¿será esto necesario?) */
};
enum consts {
    INSTS_MAX = 128,
};

struct cpu {
    int64_t regs[REG_LIMIT];
    enum cpu_state state;
    /*
     * La memoria para las instrucciones será un arreglo de 128, esperando que
     * ningún programa se acerque a esto.
     * TODO: implementar arreglo dinámico para instrucciones
     */
    struct inst instmem[INSTS_MAX];
};
struct cpu *cpu_new(void);
int32_t cpu_reset(struct cpu *self);
int32_t cpu_load_insts_from_file(struct cpu *self, const char *filename);
int32_t cpu_load_insts_from_str(struct cpu *self, char *str);
int32_t cpu_next_cycle(struct cpu *self);

#endif
