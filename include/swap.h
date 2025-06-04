#ifndef SWAP_H
#define SWAP_H

#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include "lista.h"

// Constantes para la memoria SWAP
#define SWAP_SIZE 65536       // Total de instrucciones en SWAP
#define FRAME_SIZE 16         // Instrucciones por marco
#define MAX_FRAMES 4096     // SWAP_SIZE / FRAME_SIZE
#define INSTR_SIZE 8         // Bytes por instrucción

// Entrada en la Tabla de Mapa de Swap (TMS)
typedef struct {
    int pid;                  // PID del proceso dueño (-1 si libre)
} FrameEntry;

// Prototipos de funciones
void swap_init();
void swap_close();
int swap_allocate_frames(PCB *pcb);
void swap_free_frames(PCB *pcb);
bool swap_load_program(PCB *pcb, const char *filename);
int32_t swap_load_program1(PCB *pcb, const char *filename);
struct inst swap_get(uint16_t frame, uint8_t offset);
long swap_translate_address(PCB *pcb, int virtual_address);
int swap_calculate_frames(int program_size);
bool swap_display_frame(WINDOW *win, int frame_num);
void swap_display_map(WINDOW *win);
int swap_get_free_frame_count();
bool has_brothers(PCB *pcb, Lista *listos, Lista *ejecucion);

#endif