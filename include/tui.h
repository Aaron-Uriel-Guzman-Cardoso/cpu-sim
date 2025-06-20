#ifndef TUI_H
#define TUI_H

void tui_init(void);
bool tui_input_handler(void);
int32_t tui_curr_proc_update(void);
void tui_processes_update(void);

#endif // TUI_H