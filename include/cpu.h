#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>
#include <insts.h>
#include <time.h>

/*
 * Posibles sucesos que pueden ocurrir durante la ejecución de la CPU.
 * Estos indican eventos de utilidad para el sistema operativo que serán
 * manejados por este mismo.
 */
enum cpu_event {
    CPU_INSTRUCTION_EXECUTED,
    CPU_INSTRUCTION_ILEGAL,     /* TODO: implementar funcionamiento */
    CPU_INSTRUCTION_INVALID,    /* TODO: implementar funcionamiento */
    CPU_REGISTER_OVERFLOW,      /* TODO: implementar funcionamiento */
    CPU_DIVISION_BY_ZERO,       /* TODO: implementar funcionamiento */
    CPU_HALT,
    CPU_NONE
};
enum consts {
    INSTS_MAX = 128,
};

/*
 * Estructura que representa el estado de la CPU en un momento dado, esta
 * estructura será símplemente una copia de todos los registros de la CPU.
 */
struct cpu_context {
    int64_t regs[REG_LIMIT];
};

struct cpu {
    struct timespec last_cycle;  /**< Última vez que CPU hizo un ciclo de reloj */
    struct timespec target_freq; /**< Frecuencia objetivo en segundos */
    bool halt;                   /**< Si se ejecutó la instrucción END */
    bool div_by_zero;            /**< Ocurrió una división por cero en el ciclo
                                      actual */
    bool overflow;               /**< Ocurrió un desbordamiento de registro en 
                                      el ciclo actual */
    int64_t regs[REG_LIMIT];
    struct queue *events;        /**< Cola con todos los eventos que no se han consultado
                                      de la CPU */
    /*
     * La memoria para las instrucciones será un arreglo de 128, esperando que
     * ningún programa se acerque a esto.
     * TODO: implementar arreglo dinámico para instrucciones
     */
    struct inst instmem[INSTS_MAX];
};

struct cpu;
struct cpu *cpu_new(void);
int32_t cpu_reset(struct cpu *self);
int32_t cpu_load_insts_from_file(struct cpu *self, const char *filename);
int32_t cpu_load_insts_from_str(struct cpu *self, char *str);
struct cpu_context cpu_dump_context(struct cpu *self);
void cpu_set_freq(struct cpu *self, double freq);
double cpu_get_freq(struct cpu *self);
int32_t cpu_sync(struct cpu *self);
enum cpu_event cpu_poll_event(struct cpu *self);
int32_t cpu_load_from_context(struct cpu *self, struct cpu_context context, struct inst instmem[128]);

#endif
