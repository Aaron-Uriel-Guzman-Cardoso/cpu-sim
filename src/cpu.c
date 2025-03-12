#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <msg.h>
#include <cpu.h>
#include <insts.h>

struct cpu *
cpu_new(void)
{
    struct cpu *new_cpu = calloc(1, sizeof(*new_cpu));
    if (new_cpu) {
        new_cpu->state = CPU_HALT;
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
    self->state = CPU_HALT;
    memset(self->instmem, 0, sizeof(self->instmem));
    memset(self->regs, 0, sizeof(self->regs));
    return 0;
}

/*
 * Carga un archivo de instrucciones para ser ejecutada por la CPU.
 * Básicamente inicializa un archivo como memoria de ejecución de
 * instrucciones.
 */
int32_t
cpu_load_insts_from_file(struct cpu *self, const char *filename)
{
    if (!self || !filename) { return -1; }
    FILE *instfile = fopen(filename, "r");
    if (!instfile) {
        return 2;
    }
    size_t instmem_end = 0;
    char buf[32];
    bool end_found = false;
    while (fgets(buf, sizeof(buf), instfile) && !end_found) {
        if (instmem_end > INSTS_MAX) {
            /*
             * Nos pasamos del límite de instrucciones :(
             */
            return 2;
        }
        for (size_t i = 0; buf[i] != '\0'; i += 1) {
            buf[i] = toupper(buf[i]);
        }
        /*
         * TODO: factorizar el código de modo que no tengamos que
         * estar usando malloc seguido de free para las instrucciones.
         */
        struct inst *tmp = inst_from_str(buf);
        if (tmp->op == OP_END) {
            end_found = true;
        }
        if (!tmp) {
            char logstr[50];
            /*
             * La forma en que registra el error es distinta para el front-end
             * y la CPU, ahorita imprimimos en stderr para simplicidad.
             * TODO: definir como manejaremos las impresiones desde la CPU de
             * forma que sea compatible tanto en pruebas unitarias como en el
             * front-end.
             */
            sprintf(logstr, "Instrucción \"%s\" inválida, remplazada por END\n",
                    buf);
            msg_log(LOG_LEVEL_WARN, logstr);
            tmp = inst_from_str("END");
            self->instmem[instmem_end] = *tmp;
            free(tmp);
            return 1;
        }
        self->instmem[instmem_end] = *tmp;
        free(tmp);
        instmem_end += 1;
    }
    self->state = CPU_READY;
    /*
     * Recemos porque esta magia negra funcione, estamos copiando la primera
     * instrucción de instmem a IR confiando en que struct inst es de un
     * tamaño menor y cabe en IR.
     * Esta verificación de assert debería ser estática no de ejecución
     * TODO: verificar este assert en compilación.
     */
    assert(sizeof(self->instmem[0]) <= sizeof(self->regs[REG_IR]));
    memcpy(&self->regs[REG_IR], &self->instmem[0], sizeof(self->instmem[0]));
    return 0;
}

int32_t
cpu_load_insts_from_str(struct cpu *self, char *str)
{
    if (!self || !str) { return -1; }
    for (size_t i = 0; str[i] != '\0'; i += 1) {
        str[i] = toupper(str[i]);
    }
    size_t instmem_end = 0;
    char *str_state;
    char *whole_inst_tok = strtok_r(str, "\n", &str_state);
    if (!whole_inst_tok) {
        /*
         * Formato inválido de programa de instrucciones
         */
        return 2;
    }
    struct inst *tmp = inst_from_str(whole_inst_tok);
    self->instmem[instmem_end] = *tmp;
    free(tmp);
    if (instmem_end > INSTS_MAX) {
        /*
         * Nos pasamos del límite de instrucciones :(
         */
        return 2;
    }
    instmem_end += 1;
    /*
     * Cuando encontremos END dejaremos de leer nuevas instrucciones
     */
    bool end_found = false;
    while ((whole_inst_tok = strtok_r(NULL, "\n", &str_state)) && !end_found) {
        if (strncmp(whole_inst_tok, "END", 3) == 0) {
            end_found = true;
        }
        struct inst *tmp = inst_from_str(whole_inst_tok);
        if (tmp->op == OP_END) {
            end_found = true;
        }
        if (!tmp) {
            char logstr[50];
            /*
             * La forma en que registra el error es distinta para el front-end
             * y la CPU, ahorita imprimimos en stderr para simplicidad.
             * TODO: definir como manejaremos las impresiones desde la CPU de
             * forma que sea compatible tanto en pruebas unitarias como en el
             * front-end.
             */
            sprintf(logstr, "Instrucción \"%s\" inválida, remplazada por END\n",
                    whole_inst_tok);
            msg_log(LOG_LEVEL_WARN, logstr);
            tmp = inst_from_str("END");
            self->instmem[instmem_end] = *tmp;
            free(tmp);
            return 1;
        }
        self->instmem[instmem_end] = *tmp;
        free(tmp);
        instmem_end += 1;
        if (instmem_end > INSTS_MAX) {
            /*
             * Nos pasamos del límite de instrucciones :(
             */
            return 2;
        }
    } 
    self->state = CPU_READY;

    assert(sizeof(self->instmem[0]) <= sizeof(self->regs[REG_IR]));
    memcpy(&self->regs[REG_IR], &self->instmem[0], sizeof(self->instmem[0]));
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
    /* Cargamos la instrucción de IR para su ejecución */
    struct inst *inst = (struct inst *)&self->regs[REG_IR];
    if (inst) {
        if (inst_execute(inst, self) == 1)  {
            return 3;
        }
        self->regs[REG_PC] += 1;
    } else {
        return 1;
    }
    if ((self->regs[REG_PC] - 1) < INSTS_MAX) {
        /* Cargamos la siguiente instrucción a ejecutar en el IR */
        memcpy(&self->regs[REG_IR], &self->instmem[self->regs[REG_PC]],
               sizeof(self->instmem[REG_PC]));
    } else {
        return 2;
    }
    return 0;
}
