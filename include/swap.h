#ifndef SWAP_H
#define SWAP_H

#include <stdio.h>
#include <stdbool.h>
#include "lista.h"  

// Constantes
#define SWAP_SIZE 65536       
#define FRAME_SIZE 16         
#define TOTAL_FRAMES 4096     
#define INSTR_SIZE 8   

// Tabla de Mapa de Swap (TMS)
typedef struct {
    int pid;                  
    int referenced;                   
} FrameEntry;

void swap_init();
void swap_close();
int swap_allocate_frames(PCB *pcb);
void swap_free_frames(int pid);
void swap_load_program(PCB *pcb, const char *filename);
void swap_fetch_instruction(PCB *pcb, char *instr_buffer);
long swap_translate_address(PCB *pcb, int virtual_address);
int swap_calculate_frames(int program_size);
bool swap_display_frame(WINDOW *win, int frame_num);
void swap_display_map(WINDOW *win);
int swap_get_free_frame_count();

#endif