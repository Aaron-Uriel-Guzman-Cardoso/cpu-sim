#include <ncurses.h>
#include <swap.h>
#include <insts.h>

#define MAX_FRAMES_PER_PAGE 2
#define MAX_PG MAX_FRAMES / MAX_FRAMES_PER_PAGE

struct swap_disp_state {
    WINDOW *swap_margin;
    WINDOW *swap;
    uint16_t curr_pg;
} st;

void
swap_disp_init(void)
{
    st.swap_margin = newwin(18, 90, 25, 120);
    st.swap = newwin(16, 88, 26, 121);
    box(st.swap_margin, 0, 0);
    mvwprintw(st.swap_margin, 0, 1, "Swap");
    wrefresh(st.swap_margin);
    wrefresh(st.swap);
}

void
swap_disp_update(void)
{
    werase(st.swap);
    for (uint8_t curr_rel_frame = 0; curr_rel_frame < MAX_FRAMES_PER_PAGE; curr_rel_frame += 1) {
        uint16_t frame_num = st.curr_pg * MAX_FRAMES_PER_PAGE + curr_rel_frame;
        if (frame_num >= MAX_FRAMES) {
            break;
        }
        const uint16_t horizontal_offset = curr_rel_frame * 44;
        for (uint8_t frame_offset = 0; frame_offset < FRAME_SIZE; frame_offset += 1) {
            struct inst inst = swap_get(st.curr_pg * 2, frame_offset);
            char instbuf[20];
            inst_to_str(&inst, instbuf, sizeof(instbuf));
            mvwprintw(st.swap, frame_offset, horizontal_offset, "%s", instbuf);
        }
    }
}

void
swap_disp_pg_dn(void)
{
    if (st.curr_pg > 0) {
        st.curr_pg -= 1;
    } else {
        st.curr_pg = MAX_PG - 1;
    }
}

void
swap_pg_up(void)
{
    if (st.curr_pg < MAX_PG - 1) {
        st.curr_pg += 1;
    } else {
        st.curr_pg = 0;
    }
}