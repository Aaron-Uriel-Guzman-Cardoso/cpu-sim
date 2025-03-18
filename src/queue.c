#include <queue.h>
#include <stdlib.h>

struct queue *
queue_new(void)
{
    struct queue *self = malloc(sizeof(struct queue));
    for (int32_t i = 0; i < 128; i += 1) {
        self->events[i] = CPU_NONE;
    }
    self->head = 0;
    self->tail = 0;
    return self;
}

/*
 * \brief Encola un evento en la cola.
 * 
 * Nota: si la cola está llena, los eventos se empiezan a sobreescribir.
 */
void
queue_enqueue(struct queue *self, enum cpu_event event)
{
    if (!self || event >= CPU_NONE) {
        return;
    }
    self->events[self->tail] = event;
    self->tail = (self->tail + 1) % 128;
}

enum cpu_event
queue_dequeue(struct queue *self)
{
    if (!self) {
        /**
         * TODO: cambiar a NULL cuando la cola sea genérica.
         */
        return CPU_NONE;
    }
    enum cpu_event event = self->events[self->head];
    self->events[self->head] = CPU_NONE;
    self->head = (self->head + 1) % 128;
    return event;
}

void
queue_free(struct queue *self)
{
    free(self);
}