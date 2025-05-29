#ifndef OS_H
#define OS_H
#include <stdint.h>

#define MAX_AVAILABLE_FRAMES 4096
#define FRAME_SIZE 16

uint32_t os_get_curr_pid();
struct PCB *os_get_proc(uint16_t pid);

#endif