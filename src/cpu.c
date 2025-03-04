#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <cpu.h>
#include <insts.h>

struct cpu *
cpu_new(void)
{
    struct cpu *new_cpu = calloc(1, sizeof(*new_cpu));
    if (new_cpu) {
        new_cpu->state = CPU_EXECUTING;
    }
    return new_cpu;
}

/*
 * Inicializa los valores de la CPU para poder ejecutar un nuevo programa.
 * Esto es usado principalmente cuando queramos hacer que la CPU cargue un
 * nuevo programa.
 */
int32_t
cpu_reset(struct cpu *self)
{
    self->state = CPU_EXECUTING;
    memset(self->instmem, 0, sizeof(self->instmem));
    memset(self->int_regs, 0, sizeof(self->int_regs));
    return 0;
}

/*
 * Carga un archivo de instrucciones para ser ejecutada futuramente.
 * Básicamente inicializa un archivo como memoria de ejecución de
 * instrucciones.
 * Esta función no realiza ninguna tarea de verificación sintáctica, dadas las
 * especificaciones, no podemos verificar sintácticamente antes de ejecutar
 * (aunque yo quisiera).
 */
int32_t
cpu_load_insts_from_file(struct cpu *self, const char *filename)
{
    if (!self || !filename) { return -1; }
    FILE *instfile = fopen(filename, "r");
    if (instfile) {
        size_t instmem_size = 0;
        while (feof(instfile)) {
            char buf[32];
            if (instmem_size > INSTS_MAX) {
                /*
                 * Nos pasamos del límite de instrucciones :(
                 */
                return 2;
            }
            if (fgets(buf, sizeof(buf), instfile)) {
                for (size_t i = 0; buf[i] != '\0'; i += 1) {
                    buf[i] = toupper(buf[i]);
                }
                strncpy(self->instmem[instmem_size], buf,
                        sizeof(self->instmem[instmem_size])/sizeof(char));
                if (self->instmem[instmem_size][sizeof(self->instmem[instmem_size])/sizeof(char) - 1] != '\0') {
                    return 3;
                }
                instmem_size += 1;
            }
            else {
                /*
                 * Hubo un error al leer la línea del archivo.
                 */
                return 1;
            }
        }
    }
    return 0;
}

int32_t
cpu_load_insts_from_str(struct cpu *self, char *str)
{
    if (!self || !str) { return -1; }
    for (size_t i = 0; str[i] != '\0'; i += 1) {
        str[i] = toupper(str[i]);
    }
    size_t instmem_size = 0;
    char *str_state;
    char *whole_inst_tok = strtok_r(str, "\n", &str_state);
    if (!whole_inst_tok) {
        /*
         * Formato inválido de programa de instrucciones
         */
        return 2;
    }
    strncpy(self->instmem[instmem_size], whole_inst_tok, INSTS_STR_MAX);
    /*
     * strncpy deja caracteres nulos como padding, si no hay padding
     * entonces la instrucción copiada era muy grande.
     */
    if (self->instmem[instmem_size][sizeof(self->instmem[instmem_size])/sizeof(char) - 1] != '\0') {
        return 3;
    }
    if (instmem_size > INSTS_MAX) {
        /*
         * Nos pasamos del límite de instrucciones :(
         */
        return 2;
    }
    instmem_size += 1;
    while ((whole_inst_tok = strtok_r(NULL, "\n", &str_state))) {
        if (instmem_size > INSTS_MAX) {
            /*
             * Nos pasamos del límite de instrucciones :(
             */
            return 2;
        }
        strncpy(self->instmem[instmem_size], whole_inst_tok, INSTS_STR_MAX);
        /*
         * strncpy deja caracteres nulos como padding, si no hay padding
         * entonces la instrucción copiada era muy grande.
         */
        if (self->instmem[instmem_size][INSTS_STR_MAX - 1] != '\0') {
            return 3;
        }
        instmem_size += 1;
    } 
    return 0;
}

/*
 * Ejecuta la siguiente instrucción en la memoria de instrucciones.
 * Regresa 0 cuando se decodificó correctamente una instrucción, 1 cuando una
 * instrucción no pudo ser decodificada, 2 cuando se llega al final de la
 * memoria de instrucciones (esto debería ser temporal) y 3 cuando se llega a
 * la instrucción final.
 */
int32_t
cpu_next_cycle(struct cpu *self)
{
    if (!self) { return -1; }
    if (self->state == CPU_HALT) { return 0; }
    if ((self->int_regs[REG_PC] - 1) < INSTS_MAX) {
        struct inst *inst = inst_decode(self->instmem[self->int_regs[REG_PC]]);
        if (inst) {
            if (inst_execute(inst, self) == 1)  {
                return 3;
            }
            self->int_regs[REG_PC] += 1;
        } else {
            return 1;
        }
    } else {
        return 2;
    }
    return 0;
}
