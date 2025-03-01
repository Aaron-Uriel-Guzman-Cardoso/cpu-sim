#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

//#include <cpu.h>
#include "../include/cpu.h"
//#include <insts.h>
#include "../include/insts.h"


struct cpu *
cpu_new(void)
{
    struct cpu *new_cpu = calloc(1, sizeof(*new_cpu));
    return new_cpu;
}

/*
 * Carga un archivo de instrucciones para ser ejecutada futuramente.
 * Básicamente inicializa un archivo como memoria de ejecución de
 * instrucciones
 */
int32_t
cpu_load_instfile(struct cpu *self, const char *filename)
{
    if (!self || !filename) { return -1; }
    self->instmem = fopen(filename, "r");
    if (!self->instmem) { return -1; }
    return 0;
}

/*
 * Ejecuta la siguiente instrucción en la memoria de instrucciones.
 * Regresa 0 cuando se decodificó correctamente una instrucción, 1 cuando una
 * instrucción no pudo ser decodificada y 2 cuando la instrucción decodificada
 * fue END o se detectó EOF.
 */
int32_t
cpu_next_cycle(struct cpu *self)
{
    if (feof(self->instmem)) {
        return 2;
    }
    if (!self) { return -1; }
    char buf[100];
    if (fgets(buf, sizeof(buf), self->instmem)) {
        for (size_t i = 0; buf[i] != '\0'; i += 1) {
            buf[i] = toupper(buf[i]);
        }
        struct inst *inst = inst_decode(buf);
        if (inst) {
            inst_execute(inst, self);
            self->int_regs[REG_PC] += 1;
        } else {
            return 1;
        }
    }
    return 0;
}
