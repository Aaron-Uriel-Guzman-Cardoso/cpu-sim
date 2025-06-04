#ifndef OS_H
#define OS_H
#include <stdint.h>
#include <lista.h>

#define MAX_AVAILABLE_FRAMES 4096
#define FRAME_SIZE 16

uint32_t os_get_curr_pid();
struct PCB *os_get_proc(uint16_t pid);
PCB *os_find_brother(PCB *pcb);

#endif