#ifndef PROMPT_H
#define PROMPT_H

void prompt_init(void);
void prompt_clear(void);
void prompt_insert_char(char ch);
void prompt_backspace(void);
void prompt_enter(void);
struct cmd *prompt_get_cmd(void);
void prompt_update(void);
void prompt_up(void);
void prompt_dn(void);
bool prompt_valid_cmd_is_entered(void);

#endif