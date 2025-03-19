#ifndef QUEUE_H
#define QUEUE_H

#include <cpu_event.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * /brief Cola utilizada para almacenar los eventos de la CPU.
 * 
 * TODO: Hacer que la cola sea de tamaño dinámico, por el momento es estática.
 */

struct queue {
    /**
     * TODO: hacer que la cola sea genérica para poder almacenar cualquier tipo
     */
    enum cpu_event events[128]; /**< Arreglo de eventos */
    uint32_t head;              /**< Índice de la cabeza de la cola */
    uint32_t tail;              /**< Índice de la cola de la cola */
};

struct queue *queue_new(void);
void queue_enqueue(struct queue *self, enum cpu_event event);
enum cpu_event queue_dequeue(struct queue *self);
void queue_free(struct queue *self);

#endif // QUEUE_H