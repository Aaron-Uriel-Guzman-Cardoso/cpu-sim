#include <os.h>
#include <ram.h>

/** 
 * \brief Estructura que representa a la tabla de marcos de memoria de
 *        acceso aleatorio
 */
struct tmm {
    /* ¿Qué opinan de una estructura anónima xd? */
    struct { uint16_t pid; bool touched; } rows[RAM_NUM_FRAMES]; /*< Índice es el marco */
    uint16_t clock_pos; /*< Índice usado en algoritmo del reloj */
} tmm;


/**
 * \brief Maneja los fallos de páginas identificados por el MMU
 * 
 * Esta función asume que se ha determinado un fallo de página en la RAM por
 * parte del MMU, aquí es donde se implementa el algoritmo del reloj 
 * (o de segunda oportunidad) para buscar el reemplazo de un solo marco para
 * satisfacer la necesidad de una página.
 * 
 * \param proc El proceso del cual se ha determinado que no tiene una pagina de
 *             swap cargada en RAM.
 * \param page El índice de página en la TMP del proceso en cuestión que no se
 *             encuentra cargada cargada en RAM y se va a utilizar.
 */
void
os_page_fault_handler(PCB *proc, uint16_t page)
{
    for (;
         tmm.rows[tmm.clock_pos].touched != 0;
         tmm.clock_pos = ((tmm.clock_pos + 1) % RAM_NUM_FRAMES))
    {
        if (tmm.rows[tmm.clock_pos].touched) {
            tmm.rows[tmm.clock_pos].touched = 0;
        }
    }
    uint16_t target_frame = tmm.clock_pos;
    word_t frame2load[RAM_FRAME_SIZE];
    swap_get_page(frame2load, proc->tmp_rows[page].on_swap);
    
    ram_write(ram_frame_to_addr(target_frame), frame2load, sizeof(frame2load));
}