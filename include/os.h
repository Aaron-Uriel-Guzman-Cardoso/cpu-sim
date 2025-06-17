#ifndef OS_H
#define OS_H
#include <stdint.h>
#include <lista.h>
#include <ram.h>

int32_t os_init(void);
uint32_t os_get_curr_pid();
PCB *os_find_sibling(PCB *pcb);
struct PCB *os_get_proc(uint16_t pid);
void os_page_fault_handler(PCB *proc, uint16_t page);
int32_t os_process_handler(void);

int32_t tmm_touch(uint16_t frame);

#endif