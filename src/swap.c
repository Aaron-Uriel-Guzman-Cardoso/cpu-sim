#include "swap.h"
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

// Archivo SWAP global
static FILE *swap_file = NULL;

// Tabla de Mapa de Swap (TMS) global
static FrameEntry swap_map[TOTAL_FRAMES];

// Inicializar el sistema SWAP
void swap_init() {
    // Crear/abrir archivo SWAP
    swap_file = fopen("SWAP.bin", "w+b");
    
    // Inicializar con ceros
    char zero = 0;
    for (int i = 0; i < SWAP_SIZE * INSTR_SIZE; i++) {
        fwrite(&zero, 1, 1, swap_file);
    }
    fflush(swap_file);
    
    // Inicializar TMS
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        swap_map[i].pid = -1;
        swap_map[i].referenced = 0;
    }
}

// Cerrar el sistema SWAP
void swap_close() {
    if (swap_file) {
        fclose(swap_file);
        swap_file = NULL;
    }
}

// Calcular marcos necesarios para un programa
int swap_calculate_frames(int program_size) {
    return (program_size + FRAME_SIZE - 1) / FRAME_SIZE;
}

// Asignar marcos libres a un proceso
int swap_allocate_frames(PCB *pcb) {
    int frames_needed = swap_calculate_frames(pcb->program_size);
    pcb->tmp = malloc(frames_needed * sizeof(int));
    pcb->tmp_size = 0;

    for (int i = 0; i < TOTAL_FRAMES && pcb->tmp_size < frames_needed; i++) {
        if (swap_map[i].pid == -1) {
            swap_map[i].pid = pcb->PID;
            pcb->tmp[pcb->tmp_size++] = i;
        }
    }
    
    return (pcb->tmp_size == frames_needed) ? 0 : -1;
}

// Liberar marcos de un proceso
void swap_free_frames(int pid) {
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (swap_map[i].pid == pid) {
            swap_map[i].pid = -1;
            swap_map[i].referenced = 0;
        }
    }
}

// Cargar programa en SWAP
void swap_load_program(PCB *pcb, const char *filename) {
    FILE *program = fopen(filename, "r");
    char buffer[INSTR_SIZE];
    
    for (int i = 0; i < pcb->tmp_size; i++) {
        for (int j = 0; j < FRAME_SIZE; j++) {
            if (fgets(buffer, INSTR_SIZE, program)) {
                long offset = (pcb->tmp[i] * FRAME_SIZE * INSTR_SIZE) + (j * INSTR_SIZE);
                fseek(swap_file, offset, SEEK_SET);
                fwrite(buffer, INSTR_SIZE, 1, swap_file);
            }
        }
    }
    fclose(program);
    fflush(swap_file);
}

// Traducir dirección virtual a física
long swap_translate_address(PCB *pcb, int virtual_address) {
    int frame_index = virtual_address / FRAME_SIZE;
    int offset = virtual_address % FRAME_SIZE;
    int swap_frame = pcb->tmp[frame_index];
    return (swap_frame * FRAME_SIZE + offset) * INSTR_SIZE;
}

// Leer instrucción desde SWAP
void swap_fetch_instruction(PCB *pcb, char *instr_buffer) {
    long real_address = swap_translate_address(pcb, pcb->context.regs[REG_PC]);
    fseek(swap_file, real_address, SEEK_SET);
    fread(instr_buffer, INSTR_SIZE, 1, swap_file);
    instr_buffer[INSTR_SIZE] = '\0';
}

// Verifica si un proceso tiene hermanos
bool has_brothers(PCB *pcb, Lista *listos, Lista *ejecucion) {
    PCB *current = listos->inicio;
    while (current) {
        if (current != pcb && current->UID == pcb->UID && 
            strcmp(current->fileName, pcb->fileName) == 0) {
            return true;
        }
        current = current->sig;
    }
    
    current = ejecucion->inicio;
    while (current) {
        if (current != pcb && current->UID == pcb->UID && 
            strcmp(current->fileName, pcb->fileName) == 0) {
            return true;
        }
        current = current->sig;
    }
    
    return false;
}

// Mostrar contenido de un marco
bool swap_display_frame(WINDOW *win, int frame_num) {
    char buffer[INSTR_SIZE + 1];
    werase(win);
    
    mvwprintw(win, 0, 0, "Marco %04X:", frame_num);
    for (int i = 0; i < FRAME_SIZE; i++) {
        long offset = (frame_num * FRAME_SIZE * INSTR_SIZE) + (i * INSTR_SIZE);
        fseek(swap_file, offset, SEEK_SET);
        fread(buffer, INSTR_SIZE, 1, swap_file);
        buffer[INSTR_SIZE] = '\0';
        mvwprintw(win, i+1, 0, "%02d: %s", i, buffer);
    }
    wrefresh(win);
    return true;
}

// Mostrar mapa de SWAP
void swap_display_map(WINDOW *win) {
    werase(win);
    mvwprintw(win, 0, 0, "SWAP MAP (Marcos libres: %d)", swap_get_free_frame_count());
    
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (i % 64 == 0) wprintw(win, "\n");
        wprintw(win, "%c", (swap_map[i].pid == -1) ? '.' : '0' + (swap_map[i].pid % 10));
    }
    wrefresh(win);
}

// Obtener cantidad de marcos libres
int swap_get_free_frame_count() {
    int count = 0;
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        if (swap_map[i].pid == -1) count++;
    }
    return count;
}