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

#include "../include/cpu.h"

#define PBASE 60
#define MAX_USER_STATS 256

int user_stats_count = 0;

/*struct os {
    uint32_t tms[MAX_AVAILABLE_FRAMES];
};*/
Lista *listos, *ejecucion, *terminados, *nuevos;

WINDOW *reg;
WINDOW *process;
WINDOW *counter;
WINDOW *tui_input; /* Usada para manejar toda entrada recibida*/
struct cpu *cpu; /* Tendremos una única CPU en el simulador */
struct timespec cpu_period = { 0, 500000000 }; /* Frecuencia de ejecucón de la cpu */ 

struct timespec last_process_update = { 0 };
struct timespec process_update_period = { 0, 500000000 }; // Actualizar cada 500 ms

const int32_t MAX_QUANTUM = 4;

const int32_t PBase = PBASE;
//User users[20] = {0};
//int numUs = 0;
float IncCPU = 60/MAX_QUANTUM;

struct user_control *uc;

double W = 0.0; // Peso de la CPU

/**
 * \brief Verifica si un usuario ya existe en la lista de usuarios.
 *
 * Esta función recorre la lista de usuarios y verifica si el ID de usuario
 * proporcionado ya está presente. Si el ID de usuario ya existe, retorna true,
 * de lo contrario, retorna false.
 *
 * \param uid ID del usuario a verificar.
 *
 * \return Retorna true si el usuario ya existe, false en caso contrario.
 */
/*
}*/

void process_update();

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
    clear_window_part(reg, 1, 1, 5, 78);
    struct cpu_context context = cpu_dump_context(cpu);
    mvwprintw(reg, 2, 2, "AX: %ld", context.regs[REG_AX]);
    mvwprintw(reg, 3, 2, "BX: %ld", context.regs[REG_BX]);
    mvwprintw(reg, 4, 2, "CX: %ld", context.regs[REG_CX]);
    mvwprintw(reg, 5, 2, "freq: %g Hz", cpu_get_freq(cpu));
    mvwprintw(reg, 2, 18, "DX: %ld", context.regs[REG_DX]);
    mvwprintw(reg, 3, 18, "PC: %ld", context.regs[REG_PC]);
    char current_inst[50];
    inst_to_str((struct inst *)&context.regs[REG_IR], current_inst, 50);
    mvwprintw(reg, 4, 18, "IR: %s", current_inst);

    if (ejecucion->inicio != NULL) {
        PCB *proceso = ejecucion->inicio;
        mvwprintw(reg, 1, 36, "Proceso PID: %d", proceso->PID);

        int max_lines = 7; // Espacio disponible para imprimir marcos
        int lines_used = 2; 
        for (int i = 0; i < proceso->tmp_size && lines_used < max_lines + 8; i++) {
            mvwprintw(reg, lines_used, 36, "Marco %d -> SWAP Marco %03X", i, proceso->tmp[i]);
            lines_used++;
        }

        if (lines_used == max_lines + 8) {
            mvwprintw(reg, lines_used, 36, "... (más marcos no mostrados)");
        }
    } else {
        mvwprintw(reg, 1, 36, "No hay proceso en ejecución.");
    }


    wrefresh(reg);
    return 0;
}

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
        cpu_set_freq(cpu, cpu_get_freq(cpu) / 2);
        regwin_update();
    } else if (in_ch == KEY_RIGHT || in_ch == 5) {
        cpu_set_freq(cpu, cpu_get_freq(cpu) * 2);
        regwin_update();
    }
    else {
        prompt_insert_char(in_ch);
    }
}

struct user_control *uc;

/**
 * \brief Carga un proceso en la lista de procesos listos.
 *
 * Esta función crea un nuevo nodo de tipo PCB (Process) utilizando la función `listaCreaNodo`
 * y lo inserta en la lista de procesos listos. Si el proceso se carga correctamente, se registra un mensaje
 * de log indicando que el proceso fue cargado. En caso de error, se registra un mensaje de log indicando
 * el fallo y se libera la memoria asignada al nodo si fue creado.
 *
 * \param listos Puntero a la lista de procesos listos donde se insertará el nuevo proceso.
 * \param fileName Nombre del archivo que representa el programa asociado al proceso.
 *
 * \return No devuelve ningún valor (void).
 
void cargarProceso(Lista *listos, const char *fileName) {
    uint8_t uid = 0;
    PCB *proceso = listaCreaNodo((struct cpu_context) {0}, fileName, uid);
    if (proceso != NULL && proceso->programa != NULL) {
        listaInsertarFinal(listos, proceso);
        process_update();

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

/**
 * \brief Ejecuta los procesos en la lista de ejecución y maneja los eventos de la CPU.
 *
 * Esta función se encarga de gestionar la ejecución de los procesos en la lista de ejecución.
 * Si no hay un proceso en ejecución y hay procesos en la lista de listos, mueve el primer
 * proceso de la lista de listos a la lista de ejecución. Luego, ejecuta las instrucciones
 * del proceso en ejecución y maneja los eventos generados por la CPU.
 *
 * \param quantum Puntero al contador de quantum actual.
 *
 * \return No devuelve ningún valor (void).
 */
void ejecutarProcesos(int32_t *quantum) {
    // Intentar cargar procesos desde la lista 'nuevos' a la SWAP
    if (nuevos->inicio != NULL) {
        PCB *proceso_nuevo = listaExtraeInicio(nuevos);
        if (swap_load_program1(proceso_nuevo, proceso_nuevo->fileName) == 0) {
            // Proceso cargado correctamente, mover a `listos`
            listaInsertarFinal(listos, proceso_nuevo);
            msg_log(LOG_LEVEL_INFO, "Proceso movido de Nuevos a Listos.\n");
            process_update();
            tms_disp_update();
            swap_disp_update();
        } else {
            // No se pudo cargar, devolver el proceso a `nuevos`
            listaInsertarFinal(nuevos, proceso_nuevo);
            msg_log(LOG_LEVEL_WARN, "No hay espacio en SWAP para el proceso. Permanece en Nuevos.\n");
        }
        
    }

    // Si no hay proceso en ejecución y hay procesos en listos, mover el proceso con menor prioridad a Ejecución
    if (ejecucion->inicio == NULL && listos->inicio != NULL) {
        PCB *proceso = listaExtraePrioridad(listos);
        

        listaInsertarFinal(ejecucion, proceso);
        *quantum = 0; // Reiniciar el quantum

        //print_process_frames(msg,proceso, 8)

        // Verificar si el proceso ya tiene contexto previo (fue interrumpido por quantum)
        if (proceso->context.regs[REG_PC] > 1) {
            // El proceso ya se ejecutó antes - restaurar su contexto
            cpu_reset(cpu);
            if (cpu_load_from_context(cpu, proceso->context) != 0) {
                char logstr[350];
                snprintf(logstr, sizeof(logstr), "Error al cargar el contexto del proceso %d\n", proceso->PID);
                msg_log(LOG_LEVEL_ERROR, logstr);
            } else {
                msg_log(LOG_LEVEL_INFO, "Proceso restaurado desde contexto guardado.");
            }
        }
        cpu_enable(cpu); /* Habilitamos CPU para ejecutar instrucciones, ya
                            estando lista. */
    }

    /*
     * Si hay proceso en ejecución, intentar ejecutar instrucciones.
     */
    if (ejecucion->inicio != NULL) {
        PCB *proceso = ejecucion->inicio;

        /* 
         * Cuando la CPU no ejecutó ninguna instrucción desde la última vez
         * que consultamos, no tenemos nada que hacer.
         */
        if (!cpu_sync(cpu)) {
            return;
        }

        // Manejar eventos generados por la CPU
        enum cpu_event event;
        while ((event = cpu_poll_event(cpu)) != CPU_NONE) {
            if (event == CPU_HALT) {
                // Proceso finalizado, moverlo a terminados
                PCB *finished = listaExtraeInicio(ejecucion);
                finished->KCPU += (*quantum) * IncCPU;

                // Actualizar estadísticas del usuario
                User *user = uc_get_user(uc, finished->UID);
                if (user) {
                    user->KCPUxU += (*quantum) * IncCPU;
                    assert(user->process_counter != 0);
                    user->process_counter -= 1;
                    if (user->process_counter == 0) {
                        update_user_stats(user->uid, user->KCPUxU);
                        uc_dealloc_user(uc, user->uid);
                    }
                }

                if (finished == NULL) {
                    msg_log(LOG_LEVEL_ERROR, "Error: No se pudo extraer el proceso de ejecución.\n");
                    return;
                }
                finished->context = cpu_dump_context(cpu);

                listaInsertarFinal(terminados, finished);
                swap_free_frames(finished);
                tms_disp_update();
                *quantum = 0;
                cpu_reset(cpu);
                process_update();
                return; // Salir después de manejar el evento de terminación
            }

            // Manejo de otros eventos
            if (event == CPU_INSTRUCTION_EXECUTED) {
                msg_log(LOG_LEVEL_INFO, "Instrucción ejecutada correctamente.\n");
            } else if (event == CPU_INSTRUCTION_INVALID) {
                msg_log(LOG_LEVEL_ERROR, "Instrucción inválida.\n");
            } else if (event == CPU_DIVISION_BY_ZERO) {
                char irstr[50];
                inst_to_str((struct inst *)&proceso->context.regs[REG_IR], irstr, sizeof(irstr));
                char logstr[200];
                snprintf(logstr, sizeof(logstr), "División por cero detectada: PID == %d, IR == %s, PC == %ld", 
                         proceso->PID, irstr, proceso->context.regs[REG_PC]);
                msg_log(LOG_LEVEL_WARN, logstr);
            } else if (event == CPU_REGISTER_OVERFLOW) {
                msg_log(LOG_LEVEL_WARN, "Desbordamiento de registro.\n");
            } else {
                msg_log(LOG_LEVEL_INFO, "Evento no reconocido.\n");
            }
        }

        // Incrementar el quantum porque se ejecutó una instrucción
        (*quantum)++;

        // Verificar si se superó el quantum
        if (*quantum >= MAX_QUANTUM) {
            // Quantum expirado: guardar contexto y mover el proceso de vuelta a listos
            PCB *running = listaExtraeInicio(ejecucion);
            if (running == NULL) {
                msg_log(LOG_LEVEL_ERROR, "Error: No se pudo extraer el proceso de ejecución.\n");
                return;
            }
            running->context = cpu_dump_context(cpu);
            running->KCPU += (*quantum) * IncCPU;

            // Actualizar estadísticas del usuario
            User *user = uc_get_user(uc, running->UID);
            if (user) {
                user->KCPUxU += (*quantum) * IncCPU;
                update_user_stats(user->uid, user->KCPUxU);
            }

            listaInsertarFinal(listos, running);

            // Recalcular prioridades de los procesos en la lista de listos
            PCB *current_process = listos->inicio;
            do {
                current_process->KCPU /= 2;
                User *user = uc_get_user(uc, current_process->UID);
                if (user) {
                    user->KCPUxU /= 2;
                    current_process->P = PBase + current_process->KCPU / 2 + (user->KCPUxU / (4 * uc_get_weight(uc)));
                }
            } while ((current_process = current_process->sig));

            *quantum = 0;
            cpu_reset(cpu);
            process_update();
            return; // Salir para no procesar más eventos en este ciclo
        }

        
    }
}

void counterWin(){

    clear_window_part(counter, 2, 2, 1, 76); // Clear Ejecución line
    clear_window_part(counter, 3, 2, 1, 76); // Clear Listos line
    clear_window_part(counter, 4, 2, 1, 76); // Clear Terminados line

    /*fileTitle *name;
    name = names->inicio;*/

    int count = 0;
    PCB* nodo;
    nodo = ejecucion->inicio;
    if (nodo == NULL){
        mvwprintw(counter, 2, 2, "Ejecución: ");
    }
    else{
        mvwprintw(counter, 2, 2, "Ejecución: %s", nodo->fileName);
        /*if (name == NULL){
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
        }*/
    }

    PCB* nodo2;
    nodo2 = listos->inicio;
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
    nodo3 = terminados->inicio;
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

/**
 * \brief Inicializa la ventana de visualización de procesos.
 *
 * Esta función se encarga de crear y configurar una ventana en la interfaz gráfica
 * donde se mostrará la información de los procesos (en ejecución, listos y terminados).
 *
 * \return Retorna 0 si la ventana se inicializó correctamente.
 */
int32_t
process_init(void)
{
    listos = malloc(sizeof(*listos));
    ejecucion = malloc(sizeof(*ejecucion));
    terminados = malloc(sizeof(*terminados));
    nuevos = malloc(sizeof(*nuevos));
    if (listos == NULL || ejecucion == NULL || terminados == NULL) {
        msg_log(LOG_LEVEL_ERROR, "Error al inicializar las listas de procesos.\n");
        return -1;
    }
    crearLista(listos);
    crearLista(ejecucion);
    crearLista(terminados);
    crearLista(nuevos);
    
   return 0;
}

/**
 * \brief Actualiza y muestra la información de los procesos en la interfaz gráfica.
 *
 * Esta función se encarga de actualizar la ventana de la interfaz gráfica que muestra
 * el estado de los procesos en ejecución, listos y terminados. Limpia las secciones
 * correspondientes de la ventana y luego imprime la información actualizada de cada
 * lista de procesos.
 *
 * \param ejecucion Puntero a la lista de procesos en ejecución.
 * \param listos Puntero a la lista de procesos listos para ejecutarse.
 * \param terminados Puntero a la lista de procesos terminados.
 *
 * \return No devuelve ningún valor (void).
 */
void process_update(void) {
    process = newwin(24, 125, 0, 81);
    box(process, 0, 0);
    wrefresh(process);

    // Contar archivos únicos en ejecución y listos (no terminados)
    int unique_files = 0;
    char unique_names[10][50] = {0}; // Asumimos máximo 10 nombres diferentes
    
    // 1. Procesos en ejecución
    if (ejecucion->inicio != NULL) {
        PCB *proceso = ejecucion->inicio;
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

    // 2. Procesos listos (esperando)
    PCB *actual = listos->inicio;
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

    uint8_t numUsers = uc->current_users; 
    //W = uc_get_weight(uc);

    // Mostrar el conteo de archivos únicos activos (ejecución + listos)
    mvwprintw(process, 1, 2, "Archivos únicos activos: %d   Usuarios activos: %d", unique_files, numUsers);
    mvwprintw(process, 2, 2, "------------------------------------------------|EJECUCION|-----------------------------------------------");

    // Resto de la función permanece igual...
    if (ejecucion->inicio != NULL) {
        PCB *proceso = ejecucion->inicio;
        struct cpu_context context = cpu_dump_context(cpu);
        char current_inst[50];
        inst_to_str((struct inst *)&context.regs[REG_IR], current_inst, sizeof(current_inst));
        User *user = uc_get_user(uc, proceso->UID);
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
    actual = listos->inicio;
    int fila = 5; 
    while (actual != NULL) {
        User *user = uc_get_user(uc, actual->UID);
        char str[200];
        pcb_as_str(actual, str, sizeof(str), user); 
        mvwprintw(process, fila, 2, "%s", str);
        actual = actual->sig;
        fila++;
    }

    mvwprintw(process, 6, 2, "------------------------------------------------|NUEVOS|-----------------------------------------------");
    actual = nuevos->inicio;
    int fila_nuevos = 5; 
    while (actual != NULL) {
        User *user = uc_get_user(uc, actual->UID);
        char str[200];
        pcb_as_str(actual, str, sizeof(str), user); 
        mvwprintw(process, fila_nuevos, 2, "%s", str);
        actual = actual->sig;
        fila++;
    }

    int fila_terminados = fila;
    clear_window_part(process, fila_terminados, 2, 18 - fila_terminados, 76);
    mvwprintw(process, fila_terminados, 2, "------------------------------------------------|TERMINADOS|-----------------------------------------------");
    actual = terminados->inicio;
    fila = fila_terminados + 1; 
    while (actual != NULL) {
        User *user = uc_get_user(uc, actual->UID);
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
 * \brief Regresa el PID del proceso que está manejando el sistema operativo
 * 
 * Obtenemos el identificador único del proceso actual por el que se está 
 * preocupando el SO, esto existe para exponer el estado del SO y así permitir
 * la comunicación con otros dispositivos de la computadora simulada (como el
 * MMU).
 * 
 * \returns PID de proceso en ejecución, 0 en caso de no tener procesos en
 *          ejecución (algo que no debería suceder).
 */
uint32_t
os_get_curr_pid()
{
    return (ejecucion && ejecucion->inicio)? ejecucion->inicio->PID : 0;
}

PCB *
os_find_brother(PCB *pcb) {
    PCB *current = listos->inicio;
    while (current) {
        if (current != pcb && current->UID == pcb->UID && 
            strcmp(current->fileName, pcb->fileName) == 0) {
            return current; // Retorna el primer hermano encontrado
        }
        current = current->sig;
    }
    
    current = ejecucion->inicio;
    while (current) {
        if (current != pcb && current->UID == pcb->UID && 
            strcmp(current->fileName, pcb->fileName) == 0) {
            return current; // Retorna el primer hermano encontrado
        }
        current = current->sig;
    }
    
    return NULL;
}

struct PCB *
os_get_proc(uint16_t pid)
{
    if (pid == 0) {
        return NULL; // No hay proceso con PID 0
    }
    PCB *proceso = listaBuscarPID(listos, pid);
    if (proceso) {
        return proceso;
    }

    proceso = listaBuscarPID(ejecucion, pid);
    if (proceso) {
        return proceso;
    }
    return NULL; // No se encontró el proceso
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
        liberarLista(listos);
        liberarLista(ejecucion);
        liberarLista(terminados);
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

                /*User *user_new = crearUsuario(uid);

                if (!uc_user_exists(uc, uid))
                {
                    uc_alloc_user(uc, user_new);
                }*/
               
                User *user_new;
                if (!uc_user_exists(uc, uid)) {
                    user_new = crearUsuario(uid);
                    user_new->process_counter = 0;
                    uc_alloc_user(uc, user_new);
                } else {
                    user_new = uc_get_user(uc, uid);
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
                nuevo_proceso->P = PBase;
                nuevo_proceso->KCPU = 0;
                //nuevo_proceso->KCPUxU = 0;
                

                switch(swap_load_program1(nuevo_proceso, cmd->arg1)) {
                case 1:
                    listaInsertarFinal(nuevos, nuevo_proceso);
                    msg_log(LOG_LEVEL_INFO, "Proceso en espera de carga en SWAP.\n");
                    break;
                case -1:
                    msg_log(LOG_LEVEL_ERROR, "Error: El programa es demasiado grande para la memoria SWAP.\n");
                    free(nuevo_proceso);
                    return 1;
                    break;
                case 0:
                    if (nuevo_proceso != NULL && nuevo_proceso->programa != NULL) {
                        listaInsertarFinal(listos, nuevo_proceso);
                        user_new -> process_counter += 1;
                        msg_log(LOG_LEVEL_INFO, "Proceso agregado a la lista de Listos.\n");
                        tms_disp_update();  
                        swap_disp_update();
                    }
                    break;
                case 2:
                    listaInsertarFinal(listos, nuevo_proceso);
                    msg_log(LOG_LEVEL_INFO, "Proceso hermano guardado.\n");
                    break;
                }  
                process_update();
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
            PCB *proceso = listaBuscarPID(listos, pid);

            if ((proceso = listaExtraePID(listos, pid)) != NULL) {
                proceso->context = cpu_dump_context(cpu);
                listaInsertarFinal(terminados, proceso);

                User *user = uc_get_user(uc, proceso->UID);
                if (user) {
                    assert(user->process_counter > 0);
                    user->process_counter -= 1;
                    if (user->process_counter == 0) {
                        update_user_stats(user->uid, user->KCPUxU);
                        uc_dealloc_user(uc, user->uid);
                    }
                }

                char mensaje[300];
                snprintf(mensaje, sizeof(mensaje), "Proceso eliminado: %d\n", pid);
                msg_log(LOG_LEVEL_INFO, mensaje);
                process_update();
            } else if ((proceso = listaExtraePID(ejecucion, pid)) != NULL) {
                proceso->context = cpu_dump_context(cpu);
                listaInsertarFinal(terminados, proceso);

                User *user = uc_get_user(uc, proceso->UID);
                if (user) {
                    assert(user->process_counter > 0);
                    user->process_counter -= 1;
                    if (user->process_counter == 0) {
                        update_user_stats(user->uid, user->KCPUxU);
                        uc_dealloc_user(uc, user->uid);
                    }
                }

                char mensaje[300];
                snprintf(mensaje, sizeof(mensaje), "Proceso eliminado: %d\n", pid);
                msg_log(LOG_LEVEL_INFO, mensaje);
                process_update();
            } else {
                char mensaje[300];
                snprintf(mensaje, sizeof(mensaje), "Error: No se pudo eliminar el proceso con PID %d.\n", pid);
                msg_log(LOG_LEVEL_ERROR, mensaje);
                return 1;
            }
            char mensaje[300];
            snprintf(mensaje, sizeof(mensaje), "Proceso eliminado: %d\n", pid);
            msg_log(LOG_LEVEL_INFO, mensaje);
            process_update();
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
    cpu = cpu_new();
    assert(cpu);
    swap_init();
    process_init();

    initscr();
    raw();
    noecho();
    cbreak();
    curs_set(1);
    
    tui_input_handler_init();
    reg = newwin(7, 80, 10, 0);
    box(reg, 0, 0);
    wrefresh(reg);
    regwin_update();

    msg_init();
    prompt_init();
    swap_disp_init();
    tms_disp_init();

    uc = malloc(sizeof(*uc));
    if (!uc) {
        msg_log(LOG_LEVEL_ERROR, "Error al inicializar user_control.\n");
        exit(1);
    }
    uc->current_users = 0;
    for (int i = 0; i < 256; i++) {
        uc->users[i] = NULL;
    }

    // Inicializar el quantum
    int quantum = 0;

    process_update();


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
        ejecutarProcesos(&quantum);
        regwin_update();
        //counterWin();

        /**f
         * TODO: evitar que la ventana de procesos se actualice tan seguido
         */
        process_update();

        /*struct timespec now;
        clock_gettime(CLOCK_MONOTONIC, &now);
        long elapsed_ns = (now.tv_sec - last_process_update.tv_sec) * 1000000000L +
                        (now.tv_nsec - last_process_update.tv_nsec);
        if (elapsed_ns >= process_update_period.tv_nsec) {
            process_update();
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
    liberarLista(listos);
    liberarLista(ejecucion);
    liberarLista(terminados);

    endwin();
    return 0;
}
