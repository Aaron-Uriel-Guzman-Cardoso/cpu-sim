#include <queue.h>
#include <stdlib.h>

/**
 * \brief Crea una nueva cola de eventos.
 * \return Retorna un puntero a la cola creada o NULL si falla la asignación de memoria.
 * \details Inicializa una cola de eventos con todos los valores establecidos en CPU_NONE.
 */
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

/**
 * \brief Encola un evento en la cola.
 * \param self Puntero a la cola donde se encolará el evento.
 * \param event Evento que se desea encolar.
 * \return No devuelve ningún valor (void).
 * \details Si la cola está llena, los eventos se sobrescriben.
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

/**
 * \brief Desencola un evento de la cola.
 * \param self Puntero a la cola de donde se desencolará el evento.
 * \return Retorna el evento desencolado o CPU_NONE si la cola está vacía o es inválida.
 * \details Elimina el evento más antiguo de la cola y lo retorna.
 */
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

/**
 * \brief Libera la memoria asignada para la cola.
 * \param self Puntero a la cola que se desea liberar.
 * \return No devuelve ningún valor (void).
 * \details Libera la memoria de la cola y sus recursos asociados.
 */
void
queue_free(struct queue *self)
{
    free(self);
}