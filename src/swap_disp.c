#include <ncurses.h>
#include <swap.h>
#include <insts.h>
#include <swap_disp.h>
#include <stdbool.h>

#define MAX_FRAMES_PER_PAGE 2
#define MAX_PG MAX_FRAMES / MAX_FRAMES_PER_PAGE

struct swap_disp_state {
    WINDOW *margin;
    WINDOW *win;
    uint16_t curr_pg;
} swap_disp_state;

void
swap_disp_init(void)
{
    swap_disp_state.margin = newwin(18, 90, 25, 116);
    swap_disp_state.win = newwin(16, 88, 26, 117);
    swap_disp_state.curr_pg = 0;
    box(swap_disp_state.margin, 0, 0);
    mvwprintw(swap_disp_state.margin, 0, 1, "Swap");
    wrefresh(swap_disp_state.margin);
    wrefresh(swap_disp_state.win);
    swap_disp_update();
}

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