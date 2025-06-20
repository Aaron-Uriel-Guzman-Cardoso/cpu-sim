#ifndef SWAP_H
#define SWAP_H

#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include <cpu.h>
#include "lista.h"

/*
 * Definiciones constantes para la memoria swap
 */
#define SWAP_PAGE_SIZE 16                                       // Instrucciones por página
#define SWAP_NUM_PAGES 4096                                     // Cuantas páginas tiene SWAP
#define SWAP_TOTAL_WORD_SIZE SWAP_PAGE_SIZE * SWAP_NUM_PAGES    // Total de instrucciones en SWAP
#define SWAP_TOTAL_BYTE_SIZE WORD_SIZE * SWAP_TOTAL_WORD_SIZE   // Tamaño total de SWAP en bytes

// Entrada en la Tabla de Mapa de Swap (TMS)
typedef struct {
    int pid;                  // PID del proceso dueño (-1 si libre)
} FrameEntry;

// Prototipos de funciones
void swap_init();
void swap_close();
int swap_allocate_frames(PCB *pcb);
void swap_free_frames(PCB *pcb);
int32_t swap_load_prog(PCB *pcb);
int32_t swap_get_page(word_t dst[SWAP_PAGE_SIZE], uint16_t page_index);
int32_t swap_get_pid(uint16_t frame);
long swap_translate_address(PCB *pcb, int virtual_address);
int swap_calculate_frames(int program_size);
bool swap_display_frame(WINDOW *win, int frame_num);
void swap_display_map(WINDOW *win);
int swap_get_free_frame_count();
bool has_brothers(PCB *pcb, Lista *proc_ready, Lista *proc_running);


#endif