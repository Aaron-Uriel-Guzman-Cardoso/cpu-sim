#ifndef SWAP_H
#define SWAP_H

#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include "lista.h"

/*
 * Definiciones constantes para la memoria swap
 */
#define PAGE_MAX_WORDS 16                        // Instrucciones por página
#define WORD_SIZE 8                              // Bytes por instrucción/palabra
#define SWAP_MAX_WORDS 65536                     // Total de instrucciones en SWAP
#define SWAP_MAX_PAGES SWAP_SIZE / PAGE_SIZE     // Cuantas páginas tiene SWAP
#define SWAP_SIZE WORD_SIZE * SWAP_MAX_WORDS     // Tamaño total de SWAP en bytes

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
int32_t swap_get_page(word_t dst[PAGE_MAX_WORDS], uint16_t page_index);
int32_t swap_get_pid(uint16_t frame);
long swap_translate_address(PCB *pcb, int virtual_address);
int swap_calculate_frames(int program_size);
bool swap_display_frame(WINDOW *win, int frame_num);
void swap_display_map(WINDOW *win);
int swap_get_free_frame_count();
bool has_brothers(PCB *pcb, Lista *listos, Lista *ejecucion);


#endif