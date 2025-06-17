#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <msg.h>
#include <ncurses_utilities.h>
#include <insts.h>
#include <lista.h>
#include <assert.h>
#include <os.h>
#include <swap.h>
#include <mmu.h>
#include <swap_disp.h>
#include <tms_disp.h>
#include <prompt.h>
#include <cmd.h>
#include <user_control.h>

#include "../include/cpu.h"

#define PBASE 60
#define MAX_USER_STATS 256

int user_stats_count = 0;

Lista *proc_ready, *proc_running, *proc_finished, *proc_new;

WINDOW *reg;
WINDOW *process;
WINDOW *counter;
WINDOW *tui_input; /* Usada para manejar toda entrada recibida*/

struct timespec cpu_period = { 0, 500000000 }; /* Frecuencia de ejecucón de la cpu */ 

struct timespec last_process_update = { 0 };
struct timespec process_update_period = { 0, 500000000 }; // Actualizar cada 500 ms

void process_disp_update();

/**
 * \brief Actualiza la ventana de registros de la CPU.
 *
 * Esta función se encarga de actualizar la ventana que muestra los registros de la CPU
 * y su estado actual.
 *
 * \return Retorna 0 si la actualización se realizó correctamente.
 */
int32_t
regwin_update(void)
{
    clear_window_part(reg, 1, 1, 5, 108);
    struct cpu_context context = cpu_dump_context();
    mvwprintw(reg, 2, 2, "AX: %ld", context.regs[REG_AX]);
    mvwprintw(reg, 3, 2, "BX: %ld", context.regs[REG_BX]);
    mvwprintw(reg, 4, 2, "CX: %ld", context.regs[REG_CX]);
    mvwprintw(reg, 5, 2, "freq: %g Hz", cpu_get_freq());
    mvwprintw(reg, 2, 18, "DX: %ld", context.regs[REG_DX]);
    mvwprintw(reg, 3, 18, "PC: %ld", context.regs[REG_PC]);
    char current_inst[50];
    inst_to_str((struct inst *)&context.regs[REG_IR], current_inst, 50);
    mvwprintw(reg, 4, 18, "IR: %s", current_inst);

    if (proc_running->inicio != NULL) {
        PCB *proceso = proc_running->inicio;
        mvwprintw(reg, 1, 52, "Proceso PID: %d", proceso->PID);

        int max_lines = 7; // Espacio disponible para imprimir marcos
        int lines_used = 2; 
        for (int i = 0; i < proceso->tmp_size && lines_used < max_lines + 8; i++) {
            mvwprintw(reg, lines_used, 52, "Marco %d -> SWAP Marco %03X, RAM Marco %03X",
                i,
                proceso->tmp_rows[i].on_swap,
                proceso->tmp_rows[i].on_ram);
            lines_used++;
        }

        if (lines_used == max_lines + 8) {
            mvwprintw(reg, lines_used, 52, "... (más marcos no mostrados)");
        }
    } else {
        mvwprintw(reg, 1, 52, "No hay proceso en ejecución.");
    }


    wrefresh(reg);
    return 0;
}

/**
 *  
 * \brief Inicializa la ventana de registros de la CPU.
 *
 * Esta función crea una nueva ventana para mostrar los registros de la CPU y
 * configura sus propiedades iniciales.
 *
 * \return Retorna 0 si la ventana se inicializó correctamente.
 */

void
tui_input_handler_init(void)
{
    tui_input = newwin(1, 1, 900, 900);
    scrollok(tui_input, TRUE);
    keypad(tui_input, TRUE);
    nodelay(tui_input, TRUE);
    wmove(tui_input, 0, 0);
    wrefresh(tui_input);
}

/**
* \brief Maneja todas las entradas del usuario
*
* Esta función maneja cualquier entrada dada por el usuario, dada la entrada 
* se determina si se trata de un caracter para la línea de comandos que 
* realizará una acción sobre el simulador o si se trata de una tecla de función
* que manipula el comportamiento de la TUI
*
* \return Se regresa 1 ocurrió un evento significativo, 0 si no ocurrió nada
*/
bool
tui_input_handler()
{
    char in_ch = wgetch(tui_input);
    char buffer[100];
    //snprintf(buffer, sizeof(buffer), "tui_input_handler: tecla %d", in_ch);
    //msg_log(LOG_LEVEL_INFO, buffer);
    if (in_ch == ERR) {
        return false;
    }
    if (in_ch == '\n') { 
        prompt_enter();
    } else if (in_ch == KEY_BACKSPACE || in_ch == 127 || in_ch == 8 ||
               in_ch == 7) {
        prompt_backspace();
    }
    else if (in_ch == KEY_F(5) || in_ch == 13 || in_ch == '+') {
        tms_disp_pg_up();
    }
    else if (in_ch == KEY_F(6) || in_ch == 14 || in_ch == '-') {
        tms_disp_pg_dn();
    }
    else if (in_ch == KEY_F(7) || in_ch == 15 || in_ch == '*') {
        swap_disp_pg_up();
    }
    else if (in_ch == KEY_F(8) || in_ch == 16 || in_ch == '/') {
        swap_disp_pg_dn();
    }
    else if (in_ch == KEY_UP || in_ch == 3) {
        prompt_up();
    } else if (in_ch == KEY_DOWN || in_ch == 2) {
        prompt_dn();
    } else if (in_ch == KEY_LEFT || in_ch == 4) {
        cpu_set_freq(cpu_get_freq() / 2);
        regwin_update();
    } else if (in_ch == KEY_RIGHT || in_ch == 5) {
        cpu_set_freq(cpu_get_freq() * 2);
        regwin_update();
    }
    else {
        prompt_insert_char(in_ch);
    }
}

/**
 * \brief Carga un proceso en la lista de procesos proc_ready.
 *
 * Esta función crea un nuevo nodo de tipo PCB (Process) utilizando la función `listaCreaNodo`
 * y lo inserta en la lista de procesos proc_ready. Si el proceso se carga correctamente, se registra un mensaje
 * de log indicando que el proceso fue cargado. En caso de error, se registra un mensaje de log indicando
 * el fallo y se libera la memoria asignada al nodo si fue creado.
 *
 * \param proc_ready Puntero a la lista de procesos proc_ready donde se insertará el nuevo proceso.
 * \param fileName Nombre del archivo que representa el programa asociado al proceso.
 *
 * \return No devuelve ningún valor (void).
 
void cargarProceso(Lista *proc_ready, const char *fileName) {
    uint8_t uid = 0;
    PCB *proceso = listaCreaNodo((struct cpu_context) {0}, fileName, uid);
    if (proceso != NULL && proceso->programa != NULL) {
        listaInsertarFinal(proc_ready, proceso);
        process_disp_update();

        char mensaje[300];
        snprintf(mensaje, sizeof(mensaje), "Proceso cargado: %s\n", fileName);
        msg_log(LOG_LEVEL_INFO, mensaje);

    } else {

        char mensaje[300];
        snprintf(mensaje, sizeof(mensaje), "Error al cargar el proceso: %s\n", fileName);
        msg_log(LOG_LEVEL_INFO, mensaje);

        if (proceso != NULL) {
            free(proceso);
        }
    }
}*/



/*
void counterWin(){

    clear_window_part(counter, 2, 2, 1, 76); // Clear Ejecución line
    clear_window_part(counter, 3, 2, 1, 76); // Clear Listos line
    clear_window_part(counter, 4, 2, 1, 76); // Clear Terminados line

    //fileTitle *name;
    //name = names->inicio;

    int count = 0;
    PCB* nodo;
    nodo = proc_running->inicio;
    if (nodo == NULL){
        mvwprintw(counter, 2, 2, "Ejecución: ");
    }
    else{
        mvwprintw(counter, 2, 2, "Ejecución: %s", nodo->fileName);
        //if (name == NULL){
            name->fileName = nodo->fileName;
            names->inicio = name;
            names->fin = name;
            count++;
        }
        else{
            name->next = malloc(sizeof(fileTitle));
            name = name->next;
            name->fileName = nodo->fileName;
            names->fin = name;
        }//
    }

    PCB* nodo2;
    nodo2 = proc_ready->inicio;
    if (nodo2 == NULL){
        mvwprintw(counter, 3, 2, "Listos: ");
    }
    else{
        mvwprintw(counter, 3, 2, "Listos: ");
        int i = 10;
        while (nodo2 != NULL){
            if(nodo2->sig == NULL){
                mvwprintw(counter, 3, i, "%s", nodo2->fileName);
            }
            else{
                mvwprintw(counter, 3, i, "%s ->", nodo2->fileName);
            }
            i += strlen(nodo2->fileName) + 4;
            nodo2 = nodo2->sig;
        }
    }

    PCB* nodo3;
    nodo3 = proc_finished->inicio;
    if (nodo3 == NULL){
        mvwprintw(counter, 4, 2, "Terminados: ");
    }
    else{
        mvwprintw(counter, 4, 2, "Terminados: ");
        int i = 14;
        while (nodo3 != NULL){
            if(nodo3->sig == NULL){
                mvwprintw(counter, 4, i, "%s", nodo3->fileName);
            }
            else{
                mvwprintw(counter, 4, i, "%s ->", nodo3->fileName);
            }
            i += strlen(nodo3->fileName) + 4;
            nodo3 = nodo3->sig;
        }
    }

    wrefresh(counter);
}
*/



/**
 * \brief Actualiza y muestra la información de los procesos en la interfaz gráfica.
 *
 * Esta función se encarga de actualizar la ventana de la interfaz gráfica que muestra
 * el estado de los procesos en ejecución, proc_ready y proc_finished. Limpia las secciones
 * correspondientes de la ventana y luego imprime la información actualizada de cada
 * lista de procesos.
 *
 * \param proc_running Puntero a la lista de procesos en ejecución.
 * \param proc_ready Puntero a la lista de procesos proc_ready para ejecutarse.
 * \param proc_new Puntero a la lista de procesos proc_new a la espera de pasar a proc_ready.
 * \param proc_finished Puntero a la lista de procesos proc_finished.
 *
 * \return No devuelve ningún valor (void).
 */
void 
process_disp_update(void)
{
    process = newwin(44, 125, 0, 110);
    box(process, 0, 0);
    wrefresh(process);

    // Contar archivos únicos en ejecución y proc_ready (no proc_finished)
    int unique_files = 0;
    char unique_names[10][50] = {0}; // Asumimos máximo 10 nombres diferentes
    
    // 1. Procesos en ejecución
    if (proc_running->inicio != NULL) {
        PCB *proceso = proc_running->inicio;
        bool exists = false;
        for (int i = 0; i < unique_files; i++) {
            if (strcmp(unique_names[i], proceso->fileName) == 0) {
                exists = true;
                break;
            }
        }
        if (!exists && unique_files < 10) {
            strncpy(unique_names[unique_files], proceso->fileName, 49);
            unique_files++;
        }
    }

    // 2. Procesos proc_ready (esperando)
    PCB *actual = proc_ready->inicio;
    while (actual != NULL && unique_files < 10) {
        bool exists = false;
        for (int i = 0; i < unique_files; i++) {
            if (strcmp(unique_names[i], actual->fileName) == 0) {
                exists = true;
                break;
            }
        }
        if (!exists) {
            strncpy(unique_names[unique_files], actual->fileName, 49);
            unique_files++;
        }
        actual = actual->sig;
    }
    // Mostrar el conteo de archivos únicos activos (ejecución + proc_ready)
    mvwprintw(process, 1, 2, "Archivos únicos activos: %d   Usuarios activos: %d",
              unique_files, uc_get_current_users());
    mvwprintw(process, 2, 2, "------------------------------------------------|EJECUCION|-----------------------------------------------");

    // Resto de la función permanece igual...
    if (proc_running->inicio != NULL) {
        PCB *proceso = proc_running->inicio;
        struct cpu_context context = cpu_dump_context();
        char current_inst[50];
        inst_to_str((struct inst *)&context.regs[REG_IR], current_inst, sizeof(current_inst));
        User *user = uc_get_user(proceso->UID);
        mvwprintw(process, 3, 2, 
                  "PID: %d, UID: %d, P: %d, KCPU: %.2f, KCPUxU: %.2f, File: %s, AX: %ld, BX: %ld, CX: %ld, DX: %ld, PC: %ld, IR: %s",
                  proceso->PID,
                  proceso->UID, 
                  proceso->P,
                  proceso->KCPU,
                  user_stats[proceso->UID].KCPUxU,
                  proceso->fileName, 
                  context.regs[REG_AX], 
                  context.regs[REG_BX],
                  context.regs[REG_CX], 
                  context.regs[REG_DX], 
                  context.regs[REG_PC], 
                  current_inst);
    } else {
        clear_window_part(process, 3, 2, 1, 76); 
    }


    mvwprintw(process, 4, 2, "--------------------------------------------------|LISTOS|--------------------------------------------------");
    actual = proc_ready->inicio;
    int fila = 5; 
    while (actual != NULL) {
        User *user = uc_get_user(actual->UID);
        char str[200];
        pcb_as_str(actual, str, sizeof(str)); 
        mvwprintw(process, fila, 2, "%s", str);
        actual = actual->sig;
        fila++;
    }

    mvwprintw(process, fila, 2, "--------------------------------------------------|NUEVOS|--------------------------------------------------");
    PCB *nuevo = proc_new->inicio;
    fila++;
    while (nuevo != NULL) {
        User *user = uc_get_user(nuevo->UID);
        char str[200];
        pcb_as_str(nuevo, str, sizeof(str)); 
        mvwprintw(process, fila, 2, "%s", str);
        nuevo = nuevo->sig;
        fila++;
    }

    int fila_terminados = fila;
    clear_window_part(process, fila_terminados, 2, 18 - fila_terminados, 76);
    mvwprintw(process, fila_terminados, 2, "------------------------------------------------|TERMINADOS|-----------------------------------------------");
    actual = proc_finished->inicio;
    fila = fila_terminados + 1; 
    while (actual != NULL) {
        User *user = uc_get_user(actual->UID);
        char str[200];
        float kcpuxu = get_user_stats(actual->UID);
        pcb_as_str_kcpuxu(actual, str, sizeof(str), kcpuxu); 
        mvwprintw(process, fila, 2, "%s", str);
        actual = actual->sig;
        fila++;
    }

    if (fila < 18) {
        clear_window_part(process, fila, 2, 18 - fila, 76); 
    }

    wrefresh(process); 
}

/**
 * \brief Evalúa e interpreta un comando ingresado por el usuario.
 *
 * Esta función evalúa un comando ingresado por el usuario y ejecuta la acción correspondiente,
 * como salir del programa o cargar un archivo en el PCB. Dependiendo del comando, realiza
 * diferentes operaciones y retorna un código de estado.
 *
 * \param cmd Puntero a la estructura que contiene el comando leído.
 *
 * \return Retorna:
 *   - 0 en caso de éxito.
 *   - 1 en caso de error.
 *   - -1 si el comando no es reconocido.
 *
 * \details
 * La función evalúa el comando ingresado y realiza la acción correspondiente:
 * - Si el comando es "EXIT" o "SALIR", cierra el programa.
 * - Si el comando es "LOAD", intenta cargar el archivo especificado en el PCB.
 * - Si el comando es nulo o no reconocido, muestra un mensaje de error en la ventana de mensajes.
 */
int32_t 
prompt_handle_cmd(struct cmd *cmd)
{
    if (strncmp(cmd->name, "EXIT", 4) == 0 || strncmp(cmd->name, "SALIR", 5) == 0) {
        liberarLista(proc_ready);
        liberarLista(proc_running);
        liberarLista(proc_finished);
        msg_log(LOG_LEVEL_INFO, "Saliendo del programa...\n");
        endwin();
        swap_close();
        exit(0);
    }
    else if (strncmp(cmd->name, "LOAD", 4) == 0) {
        if (cmd->arg1[0] == '\0') {
            msg_log(LOG_LEVEL_ERROR, "Falta el nombre del archivo\n");
            return 1;
        } else {
            uint8_t uid = 0;
            if (cmd->arg2[0] != '\0')
            {
                uid = atoi(cmd->arg2);
                if (uid > UINT8_MAX) {
                    msg_log(LOG_LEVEL_ERROR, "ID de usuario inválido");
                    return -1;
                }

                char mensaje[300];
                snprintf(mensaje, sizeof(mensaje), "Cargando archivo: %s\n", cmd->arg1);
                msg_log(LOG_LEVEL_INFO, mensaje);

                // Verificar si el archivo existe antes de crear el proceso
                FILE *archivo = fopen(cmd->arg1, "r");
                if (archivo == NULL) {
                    msg_log(LOG_LEVEL_ERROR, "Error: El archivo no existe o no se puede abrir.\n");
                    return 1; 
                }
                fclose(archivo);

                // Crear un nuevo proceso y lo agrega a la lista
                PCB *nuevo_proceso = listaCreaNodo((struct cpu_context) { 0 }, cmd->arg1, uid);
                
                //nuevo_proceso->KCPUxU = 0;
                

                switch(swap_load_program1(nuevo_proceso, cmd->arg1)) {
                case 1:
                    listaInsertarFinal(proc_new, nuevo_proceso);
                    msg_log(LOG_LEVEL_INFO, "Proceso en espera de carga en SWAP.\n");
                    break;
                case -1:
                    msg_log(LOG_LEVEL_ERROR, "Error: El programa es demasiado grande para la memoria SWAP.\n");
                    free(nuevo_proceso);
                    return 1;
                    break;
                case 0:
                    if (nuevo_proceso != NULL && nuevo_proceso->programa != NULL) {
                        User *user_new;
                        if (!uc_user_exists(uid)) {
                            user_new = crearUsuario(uid);
                            user_new->process_counter = 0;
                            uc_alloc_user(user_new);
                        } else {
                            user_new = uc_get_user(uid);
                        }
                        user_new->process_counter += 1;
                        nuevo_proceso->UID = user_new->uid;

                        listaInsertarFinal(proc_ready, nuevo_proceso);
                        msg_log(LOG_LEVEL_INFO, "Proceso agregado a la lista de Listos.\n");
                        tms_disp_update();  
                        swap_disp_update();
                    }
                    break;
                case 2:
                    listaInsertarFinal(proc_ready, nuevo_proceso);
                    msg_log(LOG_LEVEL_INFO, "Proceso hermano guardado.\n");
                    break;
                }  
                process_disp_update();
            }
            else
            {
                msg_log(LOG_LEVEL_ERROR, "Falta el ID de usuario");
                return -1;
            }
            
        }
    }
    else if (strncmp(cmd->name, "KILL", 4) == 0) {
        if (cmd->arg1[0] == '\0') {
            msg_log(LOG_LEVEL_ERROR, "Falta el PID del proceso a eliminar.\n");
            return 1;
        } else {

            int pid = atoi(cmd->arg1);

            PCB *proceso = listaBuscarPID(proc_ready, pid);

            swap_free_frames(proceso);

            if ((proceso = listaExtraePID(proc_ready, pid)) != NULL) {
                proceso->context = cpu_dump_context();
                listaInsertarFinal(proc_finished, proceso);

                User *user = uc_get_user(proceso->UID);
                if (user) {
                    assert(user->process_counter > 0);
                    user->process_counter -= 1;
                    if (user->process_counter == 0) {
                        update_user_stats(user->uid, user->KCPUxU);
                        uc_dealloc_user(user->uid);
                    }
                }

                char mensaje[300];
                snprintf(mensaje, sizeof(mensaje), "Proceso eliminado: %d\n", pid);
                msg_log(LOG_LEVEL_INFO, mensaje);
                process_disp_update();
            } else if ((proceso = listaExtraePID(proc_running, pid)) != NULL) {
                proceso->context = cpu_dump_context();
                listaInsertarFinal(proc_finished, proceso);

                User *user = uc_get_user(proceso->UID);
                if (user) {
                    assert(user->process_counter > 0);
                    user->process_counter -= 1;
                    if (user->process_counter == 0) {
                        update_user_stats(user->uid, user->KCPUxU);
                        uc_dealloc_user(user->uid);
                    }
                }

                char mensaje[300];
                snprintf(mensaje, sizeof(mensaje), "Proceso eliminado: %d\n", pid);
                msg_log(LOG_LEVEL_INFO, mensaje);
                process_disp_update();
            } else {
                char mensaje[300];
                snprintf(mensaje, sizeof(mensaje), "Error: No se pudo eliminar el proceso con PID %d.\n", pid);
                msg_log(LOG_LEVEL_ERROR, mensaje);
                return 1;
            }
            char mensaje[300];
            snprintf(mensaje, sizeof(mensaje), "Proceso eliminado: %d\n", pid);
            msg_log(LOG_LEVEL_INFO, mensaje);
            process_disp_update();
        }
    }
    else { 
        msg_log(LOG_LEVEL_ERROR, "Comando nulo.");
        return -1;
    }

    return 0;
}

/**
 * \brief Función principal del programa.
 *
 * Esta función inicializa la interfaz gráfica, la CPU y las listas de procesos.
 * Luego, entra en un bucle principal donde se actualiza la interfaz gráfica y se
 * ejecutan los procesos en la CPU.
 *
 * \return Retorna 0 si el programa se ejecutó correctamente.
 */
int 
main(void) {
    cpu_init();
    swap_init();
    os_init();

    initscr();
    raw();
    noecho();
    cbreak();
    curs_set(1);
    
    tui_input_handler_init();
    reg = newwin(7, 110, 10, 0);
    box(reg, 0, 0);
    wrefresh(reg);
    regwin_update();

    msg_init();
    prompt_init();
    swap_disp_init();
    tms_disp_init();

    // Inicializar el quantum
    int quantum = 0;

    //process_disp_update();

    while (true) {
        tui_input_handler();
        if (prompt_valid_cmd_is_entered()) {
            struct cmd *cmd = prompt_get_cmd();
            if (cmd == NULL) {
                msg_log(LOG_LEVEL_ERROR, "Comando nulo.\n");
                continue; // No hacer nada si el comando es nulo
            }
            int32_t result = prompt_handle_cmd(cmd);
        }

        // Ejecutar procesos
        os_process_handler();
        regwin_update();
        //counterWin();

        /**f
         * TODO: evitar que la ventana de procesos se actualice tan seguido
         */
        process_disp_update();

        /*struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        long elapsed_ns = (now.tv_sec - last_process_update.tv_sec) * 1000000000L +
                        (now.tv_nsec - last_process_update.tv_nsec);
        if (elapsed_ns >= process_update_period.tv_nsec) {
            process_disp_update();
            last_process_update = now;
        }*/

        wrefresh(counter);
        wrefresh(process);
        /*
         * La lógica del bucle principal se ejecuta cada 33 ms
         * (más o menos 30 FPS o HZ).
         */
        struct timespec update_delay = {
            .tv_sec = 0,
            .tv_nsec =33E6
        };
        clock_nanosleep(CLOCK_MONOTONIC, 0, &update_delay, NULL);
    }

    swap_close();

    // Liberar la memoria
    liberarLista(proc_ready);
    liberarLista(proc_running);
    liberarLista(proc_finished);

    endwin();
    return 0;
}
