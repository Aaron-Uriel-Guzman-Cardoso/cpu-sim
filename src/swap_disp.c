#include <ncurses.h>
#include <swap.h>
#include <insts.h>
#include <swap_disp.h>
#include <stdbool.h>

#define SWAP_PAGES_PER_VISUAL_PAGE 2
#define VISUAL_PAGES_SIZE SWAP_NUM_PAGES / SWAP_PAGES_PER_VISUAL_PAGE

struct swap_disp_state {
    WINDOW *margin;
    WINDOW *win;
    uint16_t curr_pg;
} swap_disp_state;

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

void
swap_disp_update(void)
{
    werase(swap_disp_state.win);
    for (uint8_t curr_rel_frame = 0; curr_rel_frame < SWAP_PAGES_PER_VISUAL_PAGE; curr_rel_frame += 1) {
        uint16_t page_num = swap_disp_state.curr_pg * SWAP_PAGES_PER_VISUAL_PAGE + curr_rel_frame;
        if (page_num >= SWAP_NUM_PAGES) {
            break;
        }
        word_t page[SWAP_PAGE_SIZE];
        swap_get_page(page, page_num);

        const uint16_t horizontal_offset = curr_rel_frame * 44;
        for (uint8_t page_offset = 0; page_offset < SWAP_PAGE_SIZE; page_offset += 1) {
            struct inst inst = *((struct inst *)(&page[page_offset]));
            size_t real_addr = (page_num * SWAP_PAGE_SIZE + page_offset) * WORD_SIZE;
            char instbuf[20];
            inst_to_str(&inst, instbuf, sizeof(instbuf));
            /*
             * Perdón por la magia negra que verán a continuación para ver en 
             * hexadecimal las instrucciones de la CPU.
             */
            mvwprintw(swap_disp_state.win, page_offset, horizontal_offset, "%05X: %016X %s",
                      real_addr, *((uint64_t *)&inst), instbuf);
        }
    }
    wrefresh(swap_disp_state.win);
}

void
swap_disp_pg_dn(void)
{
    if (swap_disp_state.curr_pg < VISUAL_PAGES_SIZE - 1) {
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
        swap_disp_state.curr_pg = VISUAL_PAGES_SIZE - 1;
    }
    swap_disp_update();
}