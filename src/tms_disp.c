#include <swap.h>
#include <tms_disp.h>

/*
 * Queremos un desplazamiento de página como el de Vim, que te mueve
 * Solo la mitad de los elementos de pantalla 
 */
const int8_t SCROLL_UNIT = PAGE_MAX_WORDS / 2;

struct tms_disp {
    int scroll_offset;
    WINDOW *win;
} tmp_disp;

/**
 * \brief Inicializa la ventana TMS (Tabla de Memoria de Segmentos).
 *
 * Esta función crea una nueva ventana para mostrar la Tabla de Memoria de Segmentos (TMS),
 * que muestra los marcos de memoria y sus respectivos PIDs asignados.
 */
void tms_disp_init(void) {
    tmp_disp.win = newwin(20, 16, 24, 0);
    tmp_disp.scroll_offset = 0;
    
    box(tmp_disp.win, 0, 0);
    mvwprintw(tmp_disp.win, 0, 6, "TMS");

    tms_disp_update();
    
    wrefresh(tmp_disp.win);
}

/**
 * \brief Actualiza la ventana TMS con los marcos y sus PIDs.
 *
 * Esta función limpia la ventana TMS y muestra el estado actual de los marcos
 * de memoria, mostrando el PID asignado a cada marco.
 */
void
tms_disp_update(void)
{
    werase(tmp_disp.win);
    box(tmp_disp.win, 0, 0);
    
    mvwprintw(tmp_disp.win, 0, 6, "TMS");
    mvwprintw(tmp_disp.win, 1, 1, "Marcos-PID");
    
    for (int i = 0; i < PAGE_MAX_WORDS; i++) {
        int marco_actual = tmp_disp.scroll_offset + i;
        if (marco_actual >= SWAP_MAX_PAGES) { break; }
        mvwprintw(tmp_disp.win, i+2, 1, "%03X - %d", marco_actual, swap_get_pid(marco_actual));
    }
    
    wrefresh(tmp_disp.win);
}

void
tms_disp_pg_dn(void)
{
    if (tmp_disp.scroll_offset + SCROLL_UNIT < SWAP_MAX_PAGES) {
        tmp_disp.scroll_offset += SCROLL_UNIT;
        tms_disp_update();
    }
}

void
tms_disp_pg_up(void)
{
    if (tmp_disp.scroll_offset - SCROLL_UNIT >= 0) {
        tmp_disp.scroll_offset -= SCROLL_UNIT;
        tms_disp_update();
    }
}

/**
 * \brief Asigna un PID a un marco de memoria en el TMS.
 *
 * Esta función asigna un PID a un marco de memoria específico en el TMS.
 * Si el número de marco es válido (entre 0 y 15), se asigna el PID al marco.
 *
 * \param frame_num Número del marco (0-15).
 * \param pid ID del proceso a asignar al marco.
 */
/*void tms_assign_frame(int frame_num, int pid) {
    if (frame_num >= 0 && frame_num < TOTAL_MARCOS) {
        tms_frames[frame_num] = pid;
        
        // Si el marco está visible, actualizar
        if (frame_num >= tms_scroll_offset && 
            frame_num < tms_scroll_offset + MARCOS_VISIBLES) {
            tms_disp_update();
        }
    }
}*/

/**
 * \brief Libera un marco de memoria en el TMS.
 *
 * Esta función libera un marco de memoria específico en el TMS, estableciendo su PID a 0.
 * Si el número de marco es válido (entre 0 y 15), se libera el marco.
 *
 * \param frame_num Número del marco (0-15) a liberar.
 */
/*void tms_free_frame(int frame_num) {
    if (frame_num >= 0 && frame_num < TOTAL_MARCOS) {
        tms_frames[frame_num] = 0;
        
        // Si el marco está visible, actualizar
        if (frame_num >= tms_scroll_offset && 
            frame_num < tms_scroll_offset + MARCOS_VISIBLES) {
            tms_disp_update();
        }
    }
}*/

