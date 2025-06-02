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

#include "../include/cpu.h"

#define MAX_CMD_CHARS 50
#define HISTORY_SIZE 3
#define PBASE 60
#define MAX_USER_STATS 256

UserStats user_stats[MAX_USER_STATS];
int user_stats_count = 0;


struct cmd {
    char name[MAX_CMD_CHARS];
    char arg1[MAX_CMD_CHARS];
    char arg2[MAX_CMD_CHARS];
};

struct cmd_history {
    struct cmd history[HISTORY_SIZE];
    int current;
    int size;
};

struct prompt {
    char buf[120];
    size_t buflen;
    struct cmd *decoded_inst;
    WINDOW *win;
    struct cmd_history hist;
    int hist_index;
};

enum prompt_status {
    PROMPT_STATUS_OK,
    PROMPT_STATUS_INVALID,
    PROMPT_STATUS_INSTRUCTION_DECODED
};


struct os {
    uint32_t tms[MAX_AVAILABLE_FRAMES];
};
Lista *listos, *ejecucion, *terminados, *nuevos;

WINDOW *reg;
WINDOW *process;
WINDOW *counter;
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
 * \brief Decodifica una cadena de texto en una estructura de comando.
 *
 * Esta función toma una cadena de texto (`buf`) que representa una instrucción,
 * la divide en componentes (nombre del comando y argumentos), y la almacena en
 * una estructura
 *
 * \param buf Cadena de texto que contiene la instrucción a decodificar.
 *
 * \return Retorna un puntero a la estructura `cmd` que contiene la instrucción decodificada.
 *         Retorna `NULL` si no se pudo asignar memoria para la estructura.
 */
struct cmd *
instruction_decode(const char *buf)
{
    struct cmd *inst = malloc(sizeof(*inst));
    if (inst) {
        //inst->name[0] = '\0';
        //inst->arg1[0] = '\0';
        //inst->arg2[0] = '\0';

        memset(inst->name, 0, sizeof(inst->name));
        memset(inst->arg1, 0, sizeof(inst->arg1));
        memset(inst->arg2, 0, sizeof(inst->arg2));

        sscanf(buf, "%s %s %s", inst->name, inst->arg1, inst->arg2);
        for(size_t i = 0; inst->name[i] != '\0'; i += 1) {
            inst->name[i] = toupper(inst->name[i]);
        }
        for(size_t i = 0; inst->arg1[i] != '\0'; i += 1) {
            inst->arg1[i] = toupper(inst->arg1[i]);
        }
        for(size_t i = 0; inst->arg2[i] != '\0'; i += 1) {
            inst->arg2[i] = toupper(inst->arg2[i]);
        }
    }
    return inst;
}

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
    mvwprintw(reg, 2, 35, "DX: %ld", context.regs[REG_DX]);
    mvwprintw(reg, 3, 35, "PC: %ld", context.regs[REG_PC]);
    char current_inst[50];
    inst_to_str((struct inst *)&context.regs[REG_IR], current_inst, 50);
    mvwprintw(reg, 4, 35, "IR: %s", current_inst);
    wrefresh(reg);
    return 0;
}

/**
 * \brief Actualiza el estado del prompt y maneja la entrada del usuario.
 *
 * Esta función gestiona la interacción del usuario con el prompt, capturando las teclas
 * presionadas y realizando acciones correspondientes, como decodificar instrucciones,
 * manejar el historial de comandos, ajustar la frecuencia de la CPU y actualizar la
 * interfaz gráfica.
 *
 * \param prompt Puntero a la estructura `prompt` que contiene el estado actual del prompt.
 *
 * \return Retorna un valor de tipo `enum prompt_status` que indica el estado actual del prompt:
 *         - `PROMPT_STATUS_OK`: El prompt está en un estado normal.
 *         - `PROMPT_STATUS_INSTRUCTION_DECODED`: Se ha decodificado una instrucción válida.
 *         - `PROMPT_STATUS_INVALID`: La instrucción ingresada no es válida.
 */
enum prompt_status
prompt_update(struct prompt *prompt)
{
    /**
     * TODO: independizar las cola circular del código de la interfaz de
     *       usuario, para que sea genérica y la podamos usar en otros
     *       lugares.
     */
    static char buf[200];
    static int32_t buflen = 0;
    int c;
    enum prompt_status status = PROMPT_STATUS_OK;
    if ((c = wgetch(prompt->win)) != -1) {
        if (c == '\n') { 
            clear_window_part(prompt->win, 1, 1, 5, 78);
            prompt->decoded_inst = instruction_decode(buf);
            buf[0] = buflen = 0;
            if (prompt->decoded_inst) {
                prompt->hist.history[prompt->hist.current] = *prompt->decoded_inst;
                prompt->hist.current = (prompt->hist.current + 1) % HISTORY_SIZE;
                if (prompt->hist.size < HISTORY_SIZE) {
                    prompt->hist.size++;
                }
                prompt->hist_index = prompt->hist.current;
                status = PROMPT_STATUS_INSTRUCTION_DECODED;
            } else {
                status = PROMPT_STATUS_INVALID;
            }
        } else if (c == KEY_BACKSPACE || c == 127 || c == 8) {
            if (buflen > 0) {
                buf[buflen - 1] = 0;
                buflen -= 1;
            }
        } else if (c == KEY_UP) {
            if (prompt->hist.size > 0) {
                prompt->hist_index = (prompt->hist_index - 1 + HISTORY_SIZE) % HISTORY_SIZE;
                snprintf(buf, sizeof(buf), "%s %s %s", prompt->hist.history[prompt->hist_index].name, prompt->hist.history[prompt->hist_index].arg1, prompt->hist.history[prompt->hist_index].arg2);
                buflen = strlen(buf);
            }
        } else if (c == KEY_DOWN) {
            if (prompt->hist.size > 0) {
                prompt->hist_index = (prompt->hist_index + 1) % HISTORY_SIZE;
                snprintf(buf, sizeof(buf), "%s %s %s", prompt->hist.history[prompt->hist_index].name, prompt->hist.history[prompt->hist_index].arg1, prompt->hist.history[prompt->hist_index].arg2);
                buflen = strlen(buf);
            }
        } else if (c == KEY_LEFT) {
            cpu_set_freq(cpu, cpu_get_freq(cpu) / 2);
            regwin_update();
        } else if (c == KEY_RIGHT) {
            cpu_set_freq(cpu, cpu_get_freq(cpu) * 2);
            regwin_update();
        }
         else if (buflen < 199) {
            buf[buflen] = c;
            buf[buflen + 1] = 0;
            buflen += 1;
        }
        clear_window_part(prompt->win, 5, 1, 1, 78);
        for (int i = 0; i < prompt->hist.size; i++) {
            int index = (prompt->hist.current - 1 - i + HISTORY_SIZE) % HISTORY_SIZE;
            mvwprintw(prompt->win, 4 - i, 1, "$ %s %s %s", 
                      prompt->hist.history[index].name, 
                      prompt->hist.history[index].arg1, 
                      prompt->hist.history[index].arg2);
        }
        wrefresh(prompt->win);
        mvwprintw(prompt->win, 5, 1, "$ %s", buf);
    }
    return status;
}

/**
 * \brief Simula un prompt para leer un comando.
 *
 * Esta función simula la lectura de un comando desde un prompt. Asigna memoria para
 * una estructura de tipo `cmd`, lee un comando predefinido ("LOAD PROG") y lo almacena
 * en la estructura.
 *
 * \return Retorna un puntero a la estructura `cmd` que contiene el comando leído.
 *         Retorna `NULL` si no se pudo asignar memoria para la estructura.
 */
struct cmd *
prompt(void)
{
    struct cmd *cmd = malloc(sizeof(*cmd));
    if (!cmd) { return NULL; }
    char buf[] = "LOAD PROG";
    sscanf(buf, "%s %s %s", cmd->name, cmd->arg1, cmd->arg2);
    for(int i = 0; cmd->name[i] != '\0'; i += 1) {
        cmd->name[i] = toupper(cmd->name[i]);
    }
    return cmd;
}

/**
 * \brief Evalúa e interpreta un comando ingresado por el usuario.
 *
 * Esta función evalúa un comando ingresado por el usuario y ejecuta la acción correspondiente,
 * como salir del programa o cargar un archivo en el PCB. Dependiendo del comando, realiza
 * diferentes operaciones y retorna un código de estado.
 *
 * \param cmd Puntero a la estructura que contiene la instrucción leída.
 * \param messages Puntero a la ventana donde se muestran los mensajes.
 * \param pcb Puntero a la estructura PCB donde se cargará el archivo (si corresponde).
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

struct user_control *uc;


int32_t 
eval(struct cmd *cmd) { 
    if (strncmp(cmd->name, "EXIT", 4) == 0 || strncmp(cmd->name, "SALIR", 5) == 0) {
        liberarLista(listos);
        liberarLista(ejecucion);
        liberarLista(terminados);
        msg_log(LOG_LEVEL_INFO, "Saliendo del programa...\n");
        endwin();
        printf("\n");
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
    }else if (strncmp(cmd->name, "KILL", 4) == 0) {
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
    process = newwin(42, 125, 0, 81);
    box(process, 0, 0);
    wrefresh(process);
 
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
void process_update() {
    werase(process);
    box(process, 0, 0);

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
                  user->KCPUxU,
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

    //mvwprintw(process, 6, 2, "------------------------------------------------|NUEVOS|-----------------------------------------------");

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
    initscr();
    noecho();
    cbreak();
    curs_set(1);

    msg_init();
    process_init();
    swap_init();

    struct prompt prompt;
    reg = newwin(7, 80, 10, 0);
    box(reg, 0, 0);
    wrefresh(reg);
    prompt.win = newwin(7, 80, 17, 0);
    box(prompt.win, 0, 0);
    counter = newwin(7, 80, 24, 0);
    box(counter, 0, 0);
    wrefresh(counter);
    wrefresh(prompt.win);
    nodelay(prompt.win, TRUE); 
    keypad(prompt.win, TRUE);
    prompt.hist.current = 0;
    prompt.hist.size = 0;
    prompt.hist_index = 0;
    mvwprintw(prompt.win, 0, 35, "|Prompt|");
    regwin_update();

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

    while (true) {
        enum prompt_status status = prompt_update(&prompt);
        if (status == PROMPT_STATUS_INSTRUCTION_DECODED) {
            eval(prompt.decoded_inst);
            free(prompt.decoded_inst);
            prompt.decoded_inst = NULL;
        }

        // Ejecutar procesos
        ejecutarProcesos(&quantum);
        counterWin();

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
        wrefresh(prompt.win);
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

    // Liberar la memoria
    liberarLista(listos);
    liberarLista(ejecucion);
    liberarLista(terminados);

    swap_close();

    endwin();
    return 0;
}
