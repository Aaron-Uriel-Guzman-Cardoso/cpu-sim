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
    struct queue *cpu = malloc(sizeof(struct queue));
    for (int32_t i = 0; i < 128; i += 1) {
        cpu->events[i] = CPU_NONE;
    }
    cpu->head = 0;
    cpu->tail = 0;
    return cpu;
}

/**
 * \brief Encola un evento en la cola.
 * \param cpu Puntero a la cola donde se encolará el evento.
 * \param event Evento que se desea encolar.
 * \return No devuelve ningún valor (void).
 * \details Si la cola está llena, los eventos se sobrescriben.
 */
void
queue_enqueue(struct queue *cpu, enum cpu_event event)
{
    if (!cpu || event >= CPU_NONE) {
        return;
    }
    cpu->events[cpu->tail] = event;
    cpu->tail = (cpu->tail + 1) % 128;
}

/**
 * \brief Desencola un evento de la cola.
 * \param cpu Puntero a la cola de donde se desencolará el evento.
 * \return Retorna el evento desencolado o CPU_NONE si la cola está vacía o es inválida.
 * \details Elimina el evento más antiguo de la cola y lo retorna.
 */
enum cpu_event
queue_dequeue(struct queue *cpu)
{
    if (!cpu) {
        return CPU_NONE;
    }
    
    // Verificar si la cola está vacía
    if (cpu->head == cpu->tail) {
        return CPU_NONE;
    }
    
    enum cpu_event event = cpu->events[cpu->head];
    cpu->events[cpu->head] = CPU_NONE;
    cpu->head = (cpu->head + 1) % 128;
    return event;
}

/**
 * \brief Libera la memoria asignada para la cola.
 * \param cpu Puntero a la cola que se desea liberar.
 * \return No devuelve ningún valor (void).
 * \details Libera la memoria de la cola y sus recursos asociados.
 */
void
queue_free(struct queue *cpu)
{
    free(cpu);
}