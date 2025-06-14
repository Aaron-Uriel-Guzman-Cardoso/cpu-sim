#ifndef OS_H
#define OS_H
#include <stdint.h>
#include <lista.h>
#include <ram.h>

uint32_t os_get_curr_pid();
struct PCB *os_get_proc(uint16_t pid);
PCB *os_find_brother(PCB *pcb);
void os_page_fault_handler(PCB *proc, uint16_t page);

#endif