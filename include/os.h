#ifndef OS_H
#define OS_H
#include <stdint.h>
#include <lista.h>
#include <ram.h>

/**
 * \brief Estados que puede adquirir un proceso en ejecución dentro del sistema
 *        operativo.
 * 
 * Nota: PROC_NEW debería llamarse PROC_WAITING pero el profe lo pidió así.
 */
enum proc_state { PROC_ANY, PROC_RUNNING, PROC_READY, PROC_FINISHED, PROC_NEW };

int32_t os_init(void);
uint32_t os_get_curr_pid();
PCB *os_find_sibling(PCB *pcb);
struct PCB *os_get_proc(uint16_t pid);
void os_page_fault_handler(PCB *proc, uint16_t page);
int32_t os_process_handler(void);

uint32_t os_new_proc(char *program, uint8_t uid);
int32_t os_kill_proc(uint16_t pid);

uint32_t os_num_progs(void);
uint32_t os_num_procs(void);
uint32_t os_num_users(void);
void os_procs_iter_init(enum proc_state kind);
PCB *os_procs_iter_next(void);


int32_t tmm_touch(uint16_t frame);

#endif