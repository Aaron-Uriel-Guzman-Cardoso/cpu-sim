#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>
#include <insts.h>
#include <time.h>
#include <stdbool.h>
#include <queue.h>
#include <cpu_event.h>

enum consts {
    INSTS_MAX = 128,
};

/**
 * \brief Representa el contexto de la CPU, siendo este únicamente los
 *        registros.
 * 
 * Estructura que representa el estado de la CPU en un momento dado,
 * requerida para cuando el sistema operativo decida realizar un
 * cambio de contexto.
 * 
 * TODO: hacer que las instmem también esté guardado en el contexto, pues
 *       literalmente pertenece a este.
 */
struct cpu_context {
    int64_t regs[REG_LIMIT];
};

/**
 * \brief Representación de CPU a utilizar por el sistemas operativo.
 * \warning Por la implementación actual de events, la CPU podrá recordar
 *          a lo mucho los últimos MAX_EVENTS eventos que ocurrieron.
 * TODO: Mover estructura CPU fuera del header y dejar una simple declaración
 *      `struct cpu;` para evitar modificaciones accidentales así como
 *       dependencias circulares.
 */
struct cpu {
    struct timespec last_cycle;  /**< Última vez que CPU hizo un ciclo de reloj */
    struct timespec target_freq; /**< Frecuencia objetivo en segundos */
    bool halt;                   /**< Si se ejecutó la instrucción END */
    bool div_by_zero;            /**< Ocurrió una división por cero en el ciclo
                                      actual */
    bool overflow;               /**< Ocurrió un desbordamiento de registro en 
                                      el ciclo actual */
    int64_t regs[REG_LIMIT];     /**< Registros de la CPU */
    struct queue *events;        /**< Cola con todos los eventos que no se han consultado
                                      de la CPU */
    /**
     * \brief Memoria donde se almacenan las instrucciones.
     * 
     * La memoria para las instrucciones será un arreglo de INSTS_MAX,
     * esperando que ningún programa se acerque a esto.
     * 
     * TODO: implementar arreglo dinámico para instrucciones, con el fin de
     *       soportar programas arbitrariamente largos.
     */
    struct inst instmem[INSTS_MAX];
};

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
