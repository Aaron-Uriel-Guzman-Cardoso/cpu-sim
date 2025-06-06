#include <ncurses.h>
#include <swap.h>
#include <insts.h>
#include <swap_disp.h>
#include <stdbool.h>

#define MAX_FRAMES_PER_PAGE 2
#define MAX_PG MAX_FRAMES / MAX_FRAMES_PER_PAGE

// Estructura que mantiene el estado de la visualización de la swap
struct swap_disp_state {
    WINDOW *margin;
    WINDOW *win;
    uint16_t curr_pg;
} swap_disp_state;

/**
 * \brief Inicializa la ventana de visualización de la swap.
 *
 * Crea una nueva ventana para mostrar el contenido de la swap, establece el número
 * de página actual a 0 y dibuja un borde alrededor de la ventana. Luego, llama a
 * `swap_disp_update()` para mostrar el contenido inicial.
 */
void
swap_disp_init(void)
{
    swap_disp_state.margin = newwin(18, 94, 25, 16);
    swap_disp_state.win = newwin(16, 92, 26, 17);
    swap_disp_state.curr_pg = 0;
    box(swap_disp_state.margin, 0, 0);
    mvwprintw(swap_disp_state.margin, 0, 1, "Swap");
    wrefresh(swap_disp_state.margin);
    wrefresh(swap_disp_state.win);
    swap_disp_update();
}

/**
 * \brief Actualiza la visualización de la swap.
 *
 * Limpia la ventana de swap y muestra el contenido de los marcos de memoria
 * correspondientes a la página actual. Cada página muestra un máximo de 2 marcos,
 * con 16 instrucciones por marco.
 */
void
swap_disp_update(void)
{
    werase(swap_disp_state.win);
    for (uint8_t curr_rel_frame = 0; curr_rel_frame < MAX_FRAMES_PER_PAGE; curr_rel_frame += 1) {
        uint16_t frame_num = swap_disp_state.curr_pg * MAX_FRAMES_PER_PAGE + curr_rel_frame;
        if (frame_num >= MAX_FRAMES) {
            break;
        }
        const uint16_t horizontal_offset = curr_rel_frame * 44;
        for (uint8_t frame_offset = 0; frame_offset < FRAME_SIZE; frame_offset += 1) {
            struct inst inst = swap_get(frame_num, frame_offset);
            size_t real_addr = (frame_num * FRAME_SIZE + frame_offset) * INSTR_SIZE;
            char instbuf[20];
            inst_to_str(&inst, instbuf, sizeof(instbuf));
            /*
             * Perdón por la mágia negra que verán a continuación para ver en 
             * hexadecimal las instrucciones de la CPU.
             */
            mvwprintw(swap_disp_state.win, frame_offset, horizontal_offset, "%05X: %016X %s",
                      real_addr, *((uint64_t *)&inst), instbuf);
        }
    }
    wrefresh(swap_disp_state.win);
}

/**
 * \brief Cambia a la página siguiente de la visualización de la swap.
 *
 * Si no está en la última página, avanza una página; si está en la última, regresa a la primera.
 * Luego actualiza la visualización.
 */
void
swap_disp_pg_dn(void)
{
    if (swap_disp_state.curr_pg < MAX_PG - 1) {
        swap_disp_state.curr_pg += 1;
    } else {
        swap_disp_state.curr_pg = 0;
    }
    swap_disp_update();
}

/**
 * \brief Cambia a la página anterior de la visualización de la swap.
 *
 * Si no está en la primera página, retrocede una página; si está en la primera, va a la última.
 * Luego actualiza la visualización.
 */
void
swap_disp_pg_up(void)
{
    if (swap_disp_state.curr_pg > 0) {
        swap_disp_state.curr_pg -= 1;
    } else {
        swap_disp_state.curr_pg = MAX_PG - 1;
    }
    swap_disp_update();
}