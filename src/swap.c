#include "swap.h"
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <cpu.h>

// Archivo SWAP global
static FILE *swapfile = NULL;

// Tabla de Mapa de Swap (TMS) global
static FrameEntry swap_map[MAX_FRAMES];

// Inicializar el sistema SWAP
void swap_init() {
    // Crear/abrir archivo SWAP
    swapfile = fopen("SWAP.bin", "w+b");
    
    // Inicializar con ceros
    char zero = 0;
    for (int i = 0; i < SWAP_SIZE * INSTR_SIZE; i++) {
        fwrite(&zero, 1, 1, swapfile);
    }
    fflush(swapfile);
    
    // Inicializar TMS
    for (int i = 0; i < MAX_FRAMES; i++) {
        swap_map[i].pid = -1;
        swap_map[i].referenced = 0;
    }
}

// Cerrar el sistema SWAP
void swap_close() {
    if (swapfile) {
        fclose(swapfile);
        swapfile = NULL;
    }
}

// Calcular marcos necesarios para un programa
int swap_calculate_frames(int program_size) {
    return (program_size + FRAME_SIZE - 1) / FRAME_SIZE;
}

/**
 * \brief Busca marcos libres para asignarselos a un proceso
 */
int swap_allocate_frames(PCB *pcb) {
    const int frames_needed = swap_calculate_frames(pcb->program_size);
    if (frames_needed >= MAX_FRAMES) {
        return -1;
    }
    pcb->tmp = malloc(frames_needed * sizeof(*pcb->tmp));
    pcb->tmp_size = 0;

    for (int i = 0; i < MAX_FRAMES && pcb->tmp_size < frames_needed; i++) {
        if (swap_map[i].pid == -1) {
            swap_map[i].pid = pcb->PID;
            pcb->tmp[pcb->tmp_size++] = i;
        }
    }
    
    return (pcb->tmp_size == frames_needed) ? 0 : -1;
}

// Liberar marcos de un proceso
void swap_free_frames(int pid) {
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (swap_map[i].pid == pid) {
            swap_map[i].pid = -1;
            swap_map[i].referenced = 0;
        }
    }
}

/**
 * \brief Carga el contenido del programa traducido a nuestro formato en swap
 * 
 * Lo que realiza(rá) es ejecutar una traducción de cadena a formato de bits 
 * legible para la CPU. Espera que el programa ya tenga marcos asignados en
 * memoria con `swap_allocate_frames()` y no vuelve a cargar este programa
 * si ya existe para el mismo usuario.
 * 
 * \return Si hubo algún error al cargar el programa se devuelve true
bool
swap_load_program(PCB *pcb, const char *filename)
{
    FILE *program = fopen(filename, "r");
     int32_t num_lines = count_instructions_in_file(program);
    const int INST_STR_SIZE = 32; // Tamaño máximo de una instrucción en formato de cadena
    char buffer[INST_STR_SIZE + 1];
    size_t program_size = 0;
    while (fgets(buffer, INST_STR_SIZE, program)) {
        struct inst *inst = inst_from_str(buffer);
        long real_addr = (pcb->tmp[i] * FRAME_SIZE + j) * INSTR_SIZE;
        fseek(swapfile, real_addr, SEEK_SET);
        fwrite(inst, INSTR_SIZE, 1, swapfile);
    }
    fclose(program);
    fflush(swapfile);
}*/


/**
 * \brief Cuenta la cantidad de instrucciones en un archivo de programa
 * \param filename Nombre del archivo a analizar
 * \return La cantidad de instrucciones encontradas, o -1 si hubo un error al abrir el archivo.
 */
int count_instructions_in_file(FILE *file) {
    if (!file) { return -1; }
    size_t count = 0;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), file)) {
        // Opcional: saltar líneas vacías o comentarios
        char *ptr = buffer;
        if (*ptr == '\n' || *ptr == '\0') continue;
        count++;
    }
    return count;
}

bool
swap_load_program1(PCB *pcb, const char *filename)
{
    FILE *program = fopen(filename, "r");
    int32_t num_lines = count_instructions_in_file(program);
    char buffer[33];

    pcb->program_size = num_lines;

    if (swap_allocate_frames(pcb) < 0) {
        fclose(program);
        return true; // No hay suficientes marcos disponibles
    }

    for (int i = 0; i < pcb->tmp_size; i++) {
        for (int j = 0; j < FRAME_SIZE; j++) {
            if (fgets(buffer, 33, program)) {
                struct inst *inst = inst_from_str(buffer);
                long real_addr = (pcb->tmp[i] * FRAME_SIZE + j) * INSTR_SIZE;
                fseek(swapfile, real_addr, SEEK_SET);
                fwrite(inst, INSTR_SIZE, 1, swapfile);
            }
        }
    }
    fclose(program);
    fflush(swapfile);
    return false;
}

/**
 * 
 * \brief Obtiene la palabra de la memoria swap en base a al marco absoluto
 *        y su offset.
 */
struct inst
swap_get(uint16_t frame, uint8_t offset)
{
    size_t real_addr = (frame * FRAME_SIZE + offset) * INSTR_SIZE;
    fseek(swapfile, real_addr, SEEK_SET);
    struct inst inst = { 0 };
    fread(&inst, INSTR_SIZE, 1, swapfile);
    return inst;
}

// Mostrar contenido de un marco
bool swap_display_frame(WINDOW *win, int frame_num) {
    char buffer[INSTR_SIZE + 1];
    werase(win);
    
    mvwprintw(win, 0, 0, "Marco %04X:", frame_num);
    for (int i = 0; i < FRAME_SIZE; i++) {
        long offset = (frame_num * FRAME_SIZE * INSTR_SIZE) + (i * INSTR_SIZE);
        fseek(swapfile, offset, SEEK_SET);
        fread(buffer, INSTR_SIZE, 1, swapfile);
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
    
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (i % 64 == 0) wprintw(win, "\n");
        wprintw(win, "%c", (swap_map[i].pid == -1) ? '.' : '0' + (swap_map[i].pid % 10));
    }
    wrefresh(win);
}

// Obtener cantidad de marcos libres
int swap_get_free_frame_count() {
    int count = 0;
    for (int i = 0; i < MAX_FRAMES; i++) {
        if (swap_map[i].pid == -1) count++;
    }
    return count;
}

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