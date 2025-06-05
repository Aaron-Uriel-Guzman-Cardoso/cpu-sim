#include "swap.h"
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <cpu.h>
#include <os.h>

#define TOTAL_MARCOS 4096  
#define MARCOS_VISIBLES 16

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
        swap_map[i].pid = 0;
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
        if (swap_map[i].pid == 0) {
            swap_map[i].pid = pcb->PID;
            pcb->tmp[pcb->tmp_size++] = i;
        }
    }
    
    return (pcb->tmp_size == frames_needed) ? 0 : -1;
}

// Liberar marcos de un proceso
void swap_free_frames(PCB *pcb) {
    /**
     * TODO: Verificar y modificar el PID cuando el hermano original tenga que ser   
     */
    PCB *brother = os_find_brother(pcb);
    if (brother) {
        for (int i = 0; i < brother->tmp_size; i++) {
            swap_map[i].pid = brother->PID;
        }
    }
    else {
        for (int i = 0; i < pcb->tmp_size; i++) {
            swap_map[pcb->tmp[i]].pid = 0;
        }
    }
    free(pcb->tmp);
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
    rewind(file); // Volver al inicio del archivo
    return count;
}

/**
 * \brief Intenta cargar programa en la memoria swap
 * 
 * Esta función carga un programa desde un archivo de texto a la memoria swap
 * 
 * \returns 0 si el programa se cargó correctamente, 1 si no hay suficientes
 *          marcos disponibles, -1 si el programa que se trata de cargar es
 *          más grande que la memoria swap, 2 si hay un proceso hermano.
 */
int32_t
swap_load_program1(PCB *pcb, const char *filename)
{
    PCB *brother;
    if((brother = os_find_brother(pcb)) != NULL) {
        pcb->tmp = malloc(brother->tmp_size * sizeof(*pcb->tmp));
        memcpy(pcb->tmp, brother->tmp, brother->tmp_size * sizeof(*pcb->tmp));
        pcb->tmp_size = brother->tmp_size;
        return 2;
    }
    FILE *program = fopen(filename, "r");
    if ((pcb->program_size = count_instructions_in_file(program)) >= SWAP_SIZE) {
        fclose(program);
        return -1; // El programa es demasiado grande para la memoria swap
    }
    char buffer[33];

    if (swap_allocate_frames(pcb) < 0) {
        fclose(program);
        return 1; // No hay suficientes marcos disponibles
    }

    for (int i = 0; i < pcb->tmp_size; i++) {
        for (int j = 0; j < FRAME_SIZE && j < pcb->program_size; j++) {
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
    return 0;
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


//
//
//

/**
 * \brief Inicializa la ventana TMS (Tabla de Memoria de Segmentos).
 *
 * Esta función crea una nueva ventana para mostrar la Tabla de Memoria de Segmentos (TMS),
 * que muestra los marcos de memoria y sus respectivos PIDs asignados.
 */

 void tms_init(WINDOW *tms_win) {
    //tms_win = newwin(19, 16, 24, 0); // Mismo tamaño y posición
    
    box(tms_win, 0, 0);
    mvwprintw(tms_win, 0, 6, "TMS");
    
    wrefresh(tms_win);
}

/* \brief Actualiza la ventana TMS con los marcos y sus PIDs.
 *
 * Esta función limpia la ventana TMS y muestra el estado actual de los marcos
 * de memoria, mostrando el PID asignado a cada marco.
 */

 void tms_update(WINDOW *tms_win, int tms_scroll_offset) {
    werase(tms_win);
    box(tms_win, 0, 0);
    
    mvwprintw(tms_win, 0, 6, "TMS");
    mvwprintw(tms_win, 1, 1, "Marcos-PID");
    
    for (int i = 0; i < MARCOS_VISIBLES; i++) {
        int marco_actual = tms_scroll_offset + i;
        if (marco_actual >= TOTAL_MARCOS) break;
        
        char marco[5];
        snprintf(marco, sizeof(marco), "%03X", marco_actual);
        mvwprintw(tms_win, i+2, 1, "%s - %d", marco, swap_map[marco_actual].pid);
    }
    
    wrefresh(tms_win);
}

void tms_handle_input(WINDOW *tms_win, int c, int tms_scroll_offset) {
    switch(c) {
        case KEY_F(5): // Bajar
            if (tms_scroll_offset + MARCOS_VISIBLES < TOTAL_MARCOS) {
                tms_scroll_offset += MARCOS_VISIBLES;
                tms_update(tms_win, tms_scroll_offset);
            }
            break;
            
        case KEY_F(6): // Subir
            if (tms_scroll_offset - MARCOS_VISIBLES >= 0) {
                tms_scroll_offset -= MARCOS_VISIBLES;
                tms_update(tms_win, tms_scroll_offset);
            }
            break;
    }
}

/**
 * \brief Asigna un PID a un marco de memoria en el TMS.
 *
 * Esta función asigna un PID a un marco de memoria específico en el TMS.
 * Si el número de marco es válido (entre 0 y 15), se asigna el PID al marco.
 *
 * \param frame_num Número del marco (0-15).
 * \param pid ID del proceso a asignar al marco.
 */
/*void tms_assign_frame(int frame_num, int pid) {
    if (frame_num >= 0 && frame_num < TOTAL_MARCOS) {
        tms_frames[frame_num] = pid;
        
        // Si el marco está visible, actualizar
        if (frame_num >= tms_scroll_offset && 
            frame_num < tms_scroll_offset + MARCOS_VISIBLES) {
            tms_update();
        }
    }
}*/

/**
 * \brief Libera un marco de memoria en el TMS.
 *
 * Esta función libera un marco de memoria específico en el TMS, estableciendo su PID a 0.
 * Si el número de marco es válido (entre 0 y 15), se libera el marco.
 *
 * \param frame_num Número del marco (0-15) a liberar.
 */
/*void tms_free_frame(int frame_num) {
    if (frame_num >= 0 && frame_num < TOTAL_MARCOS) {
        tms_frames[frame_num] = 0;
        
        // Si el marco está visible, actualizar
        if (frame_num >= tms_scroll_offset && 
            frame_num < tms_scroll_offset + MARCOS_VISIBLES) {
            tms_update();
        }
    }
}*/

/**
 * \brief Imprime los marcos ocupados por un proceso y sus equivalentes en SWAP.
 *
 * \param pcb Puntero al proceso (PCB) cuyos marcos se desean imprimir.
 * \param max_lines Número máximo de líneas que se pueden imprimir en la ventana.
 * \param msg Puntero a la ventana de mensajes
 */
void print_process_frames(WINDOW *msg, PCB *pcb, int max_lines) {
    if (!pcb || !pcb->tmp) {
        mvwprintw(msg, 1, 2, "Error: El proceso no tiene marcos asignados.");
        wrefresh(msg);
        return;
    }

    werase(msg); // Limpiar la ventana de mensajes
    box(msg, 0, 0);

    mvwprintw(msg, 1, 2, "Marcos ocupados por el proceso PID: %d", pcb->PID);
    mvwprintw(msg, 2, 2, "----------------------------------------");

    int lines_used = 3; // Contador de líneas usadas (incluyendo encabezados)
    for (int i = 0; i < pcb->tmp_size && lines_used < max_lines; i++) {
        mvwprintw(msg, lines_used, 2, "Marco %d -> SWAP Marco %d", i, pcb->tmp[i]);
        lines_used++;
    }

    if (lines_used == max_lines) {
        mvwprintw(msg, lines_used, 2, "... (más marcos no mostrados)");
    }

    wrefresh(msg);
}

