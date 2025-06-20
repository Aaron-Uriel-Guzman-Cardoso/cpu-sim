#include <swap.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <cpu.h>
#include <os.h>
#include <msg.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>


// Archivo SWAP global
static FILE *swapfile = NULL;

// Tabla de Mapa de Swap (TMS) global
static FrameEntry swap_map[SWAP_NUM_PAGES];

/**
 * \brief Inicializa el sistema SWAP.
 *
 * Esta función crea o abre el archivo de swap, lo inicializa con ceros.
 */
void
swap_init()
{
    // Crear/abrir archivo SWAP
    swapfile = fopen("SWAP.bin", "w+b");
    if (!swapfile) {
        msg_log(LOG_LEVEL_ERROR, "Error al abrir el archivo SWAP.bin");
        exit(EXIT_FAILURE);
    }
    
    // Inicializar con ceros
    for (int i = 0; i < SWAP_NUM_PAGES; i++) {
        word_t zeros[SWAP_PAGE_SIZE] = {0};
        fwrite(&zeros, WORD_SIZE, SWAP_PAGE_SIZE, swapfile);
    }
    fflush(swapfile);
    
    // Inicializar TMS
    for (int i = 0; i < SWAP_NUM_PAGES; i++) {
        swap_map[i].pid = 0;
    }
}

/**
 * \brief Cierra el sistema SWAP.
 *
 * Esta función cierra el archivo de swap si está abierto y libera el puntero global.
 */
void swap_close() {
    if (swapfile) {
        fclose(swapfile);
        swapfile = NULL;
    }
}

/**
 * \brief Calcula la cantidad de marcos necesarios para almacenar un programa.
 *
 * Dado el tamaño del programa en instrucciones, esta función determina cuántos
 * marcos de memoria (frames) se requieren para almacenarlo completamente.
 * Utiliza una división entera redondeando hacia arriba.
 *
 * \param program_size Número de instrucciones del programa.
 * \return Número de marcos necesarios.
 */
int swap_calculate_frames(int program_size) {
    return (program_size + SWAP_PAGE_SIZE - 1) / SWAP_PAGE_SIZE;
}

/**
 * \brief Busca marcos libres para asignárselos a un proceso.
 *
 * Asigna marcos libres de swap al proceso pcb y los marca con su PID.
 * Devuelve 0 si tuvo éxito, -1 si no hay suficientes marcos.
 */
int swap_allocate_frames(PCB *pcb) {
    const int frames_needed = swap_calculate_frames(pcb->program->length);
    if (frames_needed >= SWAP_NUM_PAGES) {
        return -1;
    }
    pcb->tmp_rows = malloc(frames_needed * sizeof(*pcb->tmp_rows));
    pcb->tmp_size = 0;

    for (int i = 0; i < SWAP_NUM_PAGES && pcb->tmp_size < frames_needed; i++) {
        if (swap_map[i].pid == 0) {
            swap_map[i].pid = pcb->PID;
            pcb->tmp_rows[pcb->tmp_size++].on_swap = i;
        }
    }

    /**
     * TODO: Hacer que esta inicialización no ocurra aquí en swap, sino en la creación
     *       del PCB.
     */
    for (int i = 0; i < pcb->tmp_size; i++) {
        pcb->tmp_rows[i].on_ram = -1; // Inicializar en -1 (no está en RAM)
    }
    
    return (pcb->tmp_size == frames_needed) ? 0 : -1;
}

/**
 * \brief Libera los marcos asignados a un proceso.
 *
 * Si el proceso tiene un hermano, reasigna los marcos a ese hermano.
 * Si no, marca los marcos como libres.
 */
void swap_free_frames(PCB *pcb) {
    /**
     * TODO: Verificar y modificar el PID cuando el hermano original tenga que ser   
     */
    PCB *brother = os_find_sibling(pcb);
    if (brother) {
        for (int i = 0; i < brother->tmp_size; i++) {
            swap_map[i].pid = brother->PID;
        }
    }
    else {
        for (int i = 0; i < pcb->tmp_size; i++) {
            swap_map[pcb->tmp_rows[i].on_swap].pid = 0;
        }
    }
    free(pcb->tmp_rows);
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
 * Esta función carga un programa desde un programa ya cargado en el simulador,
 * se trata de meter a la memoria swap tras ya haber creado su PCB, y en caso
 * de no ser posible se notifica.
 * 
 * \returns 0 si el programa se cargó correctamente, 1 si no hay suficientes
 *          marcos disponibles por ahora, -1 si el programa que se trata de 
 *          cargar es más grande que la memoria swap, 2 si hay un proceso
 *          hermano.
 */
int32_t
swap_load_prog(PCB *pcb)
{
    PCB *sibling;
    if((sibling = os_find_sibling(pcb)) != NULL) {
        pcb->tmp_rows = malloc(sibling->tmp_size * sizeof(*pcb->tmp_rows));
        memcpy(pcb->tmp_rows, sibling->tmp_rows, sibling->tmp_size * sizeof(*pcb->tmp_rows));
        pcb->tmp_size = sibling->tmp_size;
        return 2;
    }
    if (swap_allocate_frames(pcb) < 0) {
        return 1;
    }

    for (int i = 0; i < pcb->tmp_size; i++) {
        const uint32_t page_start = i * SWAP_PAGE_SIZE;
        const uint32_t real_byte_addr = page_start * WORD_SIZE;

        /**
         * Si la página en swap no se pasa del tamaño del programa,
         * la copiamos entera, caso contario, copiamos solo lo definido
         * y el resto lo llenamos con NOPs.
         */
        if (page_start + SWAP_PAGE_SIZE <= pcb->program->length) {
            fseek(swapfile, real_byte_addr, SEEK_SET);
            fwrite(&pcb->program->instmem[page_start], WORD_SIZE, SWAP_PAGE_SIZE, swapfile);
        } else {
            const uint32_t remaining_insts = pcb->program->length - page_start;
            fseek(swapfile, real_byte_addr, SEEK_SET);
            fwrite(&pcb->program->instmem[page_start], WORD_SIZE, remaining_insts, swapfile);
            struct inst nop = { .op = OP_NOP };
            for (uint32_t j = remaining_insts; j < SWAP_PAGE_SIZE; j++) {
                fwrite(&nop, WORD_SIZE, 1, swapfile);
            }
        }
    }
    fflush(swapfile);
    return 0;
}

/**
 * 
 * \brief Obtiene una página de memoria SWAP en términos absolutos
 * 
 * Esta es una función de bajo nivel utilizada principalmente por la MMU
 * para obtener una o varias instrucciones de la memoria SWAP. Como la swap 
 * verdadera no puede regresar una sola palabra, devolvemos una página
 * completa de memoria SWAP, que contiene PAGE_MAX_WORDS instrucciones.
 * 
 * Nota: no se realiza ninguna verificación de que la página esté realmente
 *       siendo usada, esta función puede regresar basura si no es usada
 *       adecuadamente.
 * Nota 2: tampoco se verifica que dst tenga el tamaño mínimo adecuado.
 * 
 * \param dst Arreglo del tipo word_t[SWAP_PAGE_SIZE] de destino donde se van
 *            a cargar todas las palabras leidas de la memoria SWAP.
 * \param page_index Índice de la página que se obtendrá de swap.
 * \return Si hubo algún problema al ejecutar: 0 si no hubo problema alguno y
 *         el resultado fue guardado en dst, 1 si la página no existe,
 *         2 si el arreglo era nulo.
 */
int32_t
swap_get_page(word_t dst[SWAP_PAGE_SIZE], uint16_t page_index)
{
    if (!dst) {
        return 2;
    }
    const uint16_t byte_addr = (page_index * SWAP_PAGE_SIZE) * WORD_SIZE;
    if (byte_addr >= SWAP_TOTAL_BYTE_SIZE) {
        return 1;
    }
    fseek(swapfile, byte_addr, SEEK_SET);
    fread(dst, sizeof(dst) * SWAP_PAGE_SIZE, 1, swapfile);
    return 0;
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
    if (frame < 0 || frame >= SWAP_NUM_PAGES) {
        return 0;
    }
    return swap_map[frame].pid;
}

// Mostrar contenido de un marco
bool swap_display_frame(WINDOW *win, int frame_num) {
    char buffer[WORD_SIZE + 1];
    werase(win);
    
    mvwprintw(win, 0, 0, "Marco %04X:", frame_num);
    for (int i = 0; i < SWAP_PAGE_SIZE; i++) {
        long offset = (frame_num * SWAP_PAGE_SIZE * WORD_SIZE) + (i * WORD_SIZE);
        fseek(swapfile, offset, SEEK_SET);
        fread(buffer, WORD_SIZE, 1, swapfile);
        buffer[WORD_SIZE] = '\0';
        mvwprintw(win, i+1, 0, "%02d: %s", i, buffer);
    }
    wrefresh(win);
    return true;
}

// Mostrar mapa de SWAP
void swap_display_map(WINDOW *win) {
    werase(win);
    mvwprintw(win, 0, 0, "SWAP MAP (Marcos libres: %d)", swap_get_free_frame_count());
    
    for (int i = 0; i < SWAP_NUM_PAGES; i++) {
        if (i % 64 == 0) wprintw(win, "\n");
        wprintw(win, "%c", (swap_map[i].pid == -1) ? '.' : '0' + (swap_map[i].pid % 10));
    }
    wrefresh(win);
}

/**
 * \brief Obtiene la cantidad de marcos libres en la memoria SWAP.
 *
 * Recorre la tabla de marcos y cuenta cuántos tienen pid == -1 (libres).
 *
 * \return Número de marcos libres.
 */
int swap_get_free_frame_count() {
    int count = 0;
    for (int i = 0; i < SWAP_NUM_PAGES; i++) {
        if (swap_map[i].pid == -1) count++;
    }
    return count;
}