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
 * Procesa una instrucción individual y la añade a la memoria de instrucciones
 * Esta función está realizada de esta forma para obtener un funcionamiento
 * genérico para cpu_load_insts_from_file y cpu_load_insts_from_str
 * Retorna: 0 si todo bien, 1 si hay error en la instrucción, 2 si es END
 */
static int32_t 
cpu_parse_and_load_inst(struct cpu *self, const char *inst_str, 
                              size_t *instmem_end)
{
    if (*instmem_end >= INSTS_MAX) {
        return -1;
    }
    struct inst *tmp = inst_from_str(inst_str);
    bool is_end = false;
    if (!tmp) {
        char logstr[50];
        sprintf(logstr, "Instrucción \"%s\" inválida, reemplazada por END\n",
                inst_str);
        msg_log(LOG_LEVEL_WARN, logstr);
        tmp = inst_from_str("END");
        if (!tmp) {
            return 1;
        }
        is_end = true;
    }

    if (tmp->op == OP_END) {
        is_end = true;
    }
    
    self->instmem[*instmem_end] = *tmp;
    free(tmp);
    (*instmem_end) += 1;
    
    return (is_end)? 2 : 0;
}

/*
 * Prepara la CPU para la ejecución una vez inicializado instmem
 * Realiza la copia de la primera instrucción en instmem a IR para que la CPU
 * esté lista. Es requerido que instmem ya esté inicializado con anterioridad
 * caso contrario esta función no hará nada útil.
 */
static void
cpu_prepare(struct cpu *self)
{
    self->state = CPU_READY;
    /** TODO: Hacer esta verificación en tiempo de compilación */
    /**
     * Esta verificación es realizada debido a que IR es un int64_t y la
     * estructura inst está hecha de forma que ocupe menos de 64 bits,
     * verificamos por si acaso que esta quepa sin problemas
     */
    assert(sizeof(self->instmem[0]) <= sizeof(self->regs[REG_IR]));
    memcpy(&self->regs[REG_IR], &self->instmem[0], sizeof(self->instmem[0]));
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
        int32_t result = cpu_parse_and_load_inst(self, buf, &instmem_end);
        if (result == 1) {
            return 1;
        } else if (result == 2) {
            end_found = true;
        }
    }
    cpu_prepare(self);
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
    int32_t result = cpu_parse_and_load_inst(self, whole_inst_tok, &instmem_end);
    if (result == 1) {
        return 1;
    } else if (result == 2) {
        cpu_prepare(self);
        return 0;
    }
    while ((whole_inst_tok = strtok_r(NULL, "\n", &str_state))) {
        result = cpu_parse_and_load_inst(self, whole_inst_tok, &instmem_end);
        if (result == 1) {
            return 1;
        } else if (result == 2) {
            break;
        }
    }
    cpu_prepare(self);
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
