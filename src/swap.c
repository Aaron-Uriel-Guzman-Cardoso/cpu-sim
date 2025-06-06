#include <swap.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <cpu.h>
#include <os.h>
#include <msg.h>
#include <time.h>
#include <unistd.h>

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
                struct inst inst;
                struct inst *loaded_inst = inst_from_str(buffer);
                if(loaded_inst){
                    inst = *loaded_inst;
                    free(loaded_inst);
                }
                else{
                    msg_log(LOG_LEVEL_ERROR, "Instrucción invalida");
                    inst.op = OP_END;
                    /*struct timespec update_delay = {
                        .tv_sec = 2,
                        .tv_nsec =0
                    };
                    clock_nanosleep(CLOCK_MONOTONIC, 0, &update_delay, NULL);*/
                    //usleep(2);
                }
                long real_addr = (pcb->tmp[i] * FRAME_SIZE + j) * INSTR_SIZE;
                fseek(swapfile, real_addr, SEEK_SET);
                fwrite(&inst, INSTR_SIZE, 1, swapfile);
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

/**
 * \brief Obtiene el PID del proceso que ocupa un marco específico
 * 
 * \param frame_num Número del marco a consultar
 * \return PID del proceso que ocupa el marco, 0 si está libre o se trató de
 *         acceder a un marco inválido.
 */
int32_t
swap_get_pid(uint16_t frame)
{
    if (frame < 0 || frame >= MAX_FRAMES) {
        return 0;
    }
    return swap_map[frame].pid;
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