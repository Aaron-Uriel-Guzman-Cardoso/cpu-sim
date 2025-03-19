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

#include "../include/cpu.h"

#define MAX_CMD_CHARS 50
#define HISTORY_SIZE 3

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

WINDOW *reg;
WINDOW *process;
struct cpu *cpu; /* Tendremos una única CPU en el simulador */
struct timespec cpu_period = { 0, 500000000 }; /* Frecuencia de ejecucón de la cpu */ 

struct timespec last_process_update = { 0 };
struct timespec process_update_period = { 0, 500000000 }; // Actualizar cada 500 ms

const int32_t MAX_QUANTUM = 5;

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

/*
 * Imprime un prompt y obtiene un comando ingresado por el usuario.
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

int32_t 
eval(struct cmd *cmd, Lista *listos) { 
    if (strncmp(cmd->name, "EXIT", 4) == 0 || strncmp(cmd->name, "SALIR", 5) == 0) {
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
            char mensaje[300];
            snprintf(mensaje, sizeof(mensaje), "Cargando archivo: %s\n", cmd->arg1);
            msg_log(LOG_LEVEL_INFO, mensaje);

            // Crear un nuevo proceso y lo agrega a la lista
            PCB *nuevo_proceso = listaCreaNodo((struct cpu_context) { 0 }, cmd->arg1);
            if (nuevo_proceso != NULL && nuevo_proceso->programa != NULL) {
                listaInsertarFinal(listos, nuevo_proceso);
                msg_log(LOG_LEVEL_INFO, "Proceso agregado a la lista de Listos.\n");
            } else {
                msg_log(LOG_LEVEL_ERROR, "Error al crear el proceso.\n");
                if (nuevo_proceso != NULL) {
                    free(nuevo_proceso); 
                }
            }
        }
    }
    else { 
        msg_log(LOG_LEVEL_ERROR, "Comando nulo.");
        return -1;
    }

    return 0;
}
/**
 * \brief Inicializa las listas de procesos listos, en ejecución y terminados.
 *
 * Esta función se encarga de inicializar tres listas diferentes que representan los estados de los procesos.                         
 *
 * \param listos Puntero a la lista de procesos listos que se inicializará.
 * \param ejecucion Puntero a la lista de procesos en ejecución que se inicializará.
 * \param terminados Puntero a la lista de procesos terminados que se inicializará.
 *
 * \return No devuelve ningún valor (void).
 */
void inicializarListas(Lista *listos, Lista *ejecucion, Lista *terminados) {
    crearLista(listos);
    crearLista(ejecucion);
    crearLista(terminados);
}

/**
 * \brief Carga un proceso en la lista de procesos listos.
 *
 * Esta función crea un nuevo nodo de tipo PCB (Processutilizando la función `listaCreaNodo`
 * y lo inserta en la lista de procesos listos. Si el proceso se carga correctamente, se registra un mensaje
 * de log indicando que el proceso fue cargado. En caso de error, se registra un mensaje de log indicando
 * el fallo y se libera la memoria asignada al nodo si fue creado.
 *
 * \param listos Puntero a la lista de procesos listos donde se insertará el nuevo proceso.
 * \param fileName Nombre del archivo que representa el programa asociado al proceso.
 *
 * \return No devuelve ningún valor (void).
 */
void cargarProceso(Lista *listos, const char *fileName) {
    PCB *proceso = listaCreaNodo((struct cpu_context) {0}, fileName);
    if (proceso != NULL && proceso->programa != NULL) {
        listaInsertarFinal(listos, proceso);

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
}

/**
 * \brief Ejecuta los procesos en la lista de listos y los mueve a la lista de ejecución.
 * \param listos Lista de procesos listos para ejecutar.
 * \param ejecucion Lista de procesos en ejecución.
 * \param terminados Lista de procesos terminados.
 */
void ejecutarProcesos(Lista *listos, Lista *ejecucion, Lista *terminados, int32_t *quantum) {
    // Si no hay proceso en ejecución y hay procesos en Listos, mover el primero a Ejecución
    if (ejecucion->inicio == NULL && listos->inicio != NULL) {
        PCB *proceso = listaExtraeInicio(listos);
        listaInsertarFinal(ejecucion, proceso);
        *quantum = 0; // Reiniciar el quantum
        cpu_reset(cpu);
        if (cpu_load_insts_from_file(cpu, proceso -> fileName) == 1) {
            cpu_reset(cpu);
            char logstr[200];
            snprintf(logstr, sizeof(logstr), "Error al cargar el archivo %s del proceso %d\n", proceso->fileName, proceso->PID);
            msg_log(LOG_LEVEL_ERROR, logstr);
        }

    }

    //Si hay proceso en ejecucion, ejecuta instruccion
    if (ejecucion -> inicio != NULL){
        PCB *proceso = ejecucion -> inicio;

        if (cpu_sync(cpu)) {
            /**
             * ¿Qué pasa cuando CPU ejecutó más procesos de los que debería?
             * Este es un caso posible con el diseño actual, y ocurre cuando
             * la frecuencia sea muy alta
             * TODO: Asegurarse que CPU no se pase del MAX_QUANTUM en altas
             *       frecuencias.
             */
            enum cpu_event event;
            while ((event = cpu_poll_event(cpu)) != CPU_NONE) {
                (*quantum)++;
                if (*quantum >= MAX_QUANTUM) {
                    struct PCB *executed_process = listaExtraeInicio(ejecucion);
                    executed_process->context = cpu_dump_context(cpu);
                    listaInsertarFinal(listos, executed_process);
                    cpu_reset(cpu);
                    *quantum = 0;
                    break;
                }
                /**
                 * Confiemos que logstr no se vaya a desbordar :)
                 */
                char logstr[200];
                if (event == CPU_INSTRUCTION_EXECUTED) {
                    msg_log(LOG_LEVEL_INFO, "Instrucción ejecutada correctamente.\n");
                } else if (event == CPU_INSTRUCTION_INVALID) {
                    msg_log(LOG_LEVEL_ERROR, "Instrucción inválida.\n");
                } else if (event == CPU_HALT) {
                    snprintf(logstr, sizeof(logstr), "Proceso %d terminado.\n", proceso->PID);
                    msg_log(LOG_LEVEL_INFO, logstr);
                    struct PCB *terminated_process = listaExtraeInicio(ejecucion);
                    listaInsertarFinal(terminados, terminated_process);
                    *quantum = 0;
                } else if (event == CPU_DIVISION_BY_ZERO) {
                    char irstr[50];
                    inst_to_str((struct inst *)&proceso->context.regs[REG_IR], irstr, sizeof(irstr));
                    snprintf(logstr, sizeof(logstr), "División por cero detectada: PID == %d, IR == %s\n, PC == %d", proceso->PID, irstr, proceso->context.regs[REG_PC]);
                    msg_log(LOG_LEVEL_WARN, logstr);
                } else if (event == CPU_REGISTER_OVERFLOW) {
                    char irstr[50];
                    inst_to_str((struct inst *)&proceso->context.regs[REG_IR], irstr, sizeof(irstr));
                    snprintf(logstr, sizeof(logstr), "Desbordamiento de registro detectado: PID == %d, IR == %s\n, PC == %d", proceso->PID, irstr, proceso->context.regs[REG_PC]);
                    msg_log(LOG_LEVEL_WARN, "Desbordamiento de registro.\n");
                } else {
                    msg_log(LOG_LEVEL_INFO, "Evento no reconocido.\n");
                }
            }
        }
    }
}

/**
 * \brief Inicializa la ventana de procesos
 */
int32_t
process_init(void)
{
    process = newwin(10, 80, 0, 85);
    box(process, 0, 0);
    wrefresh(process);
 
   return 0;
}

/**
 * \brief Actualiza la ventana de procesos
 */
void
process_update(Lista *ejecucion, Lista *listos, Lista *terminados)
{
    clear_window_part(process, 1, 2, 11, 76);

    // Mostrar el proceso en ejecución
    mvwprintw(process, 1, 2, "......PROCESADOR......");
    if (ejecucion->inicio != NULL) {
        PCB *proceso = ejecucion->inicio;
        char str[80];
        pcb_as_str(proceso, str, sizeof(str));
        mvwprintw(process, 2, 2, str);
    } else {
        clear_window_part(process, 2, 2, 4, 76); 
    }

    // Mostrar la lista de procesos listos
    mvwprintw(process, 7, 2, "......LISTA......");
    PCB *actual = listos->inicio;
    int fila = 8;
    while (actual != NULL && fila < 10) {
        char str[80];
        pcb_as_str(actual, str, sizeof(str)); 
        mvwprintw(process, fila, 2, str);
        actual = actual->sig;
        fila++;
    }
    if (fila < 10) {
        clear_window_part(process, fila, 2, 10 - fila, 76); 
    }

    // Mostrar la lista de procesos terminados
    mvwprintw(process, 9, 2, "......TERMINADOS......");
    actual = terminados->inicio;
    fila = 10;
    while (actual != NULL && fila < 18) { 
        char str[80];
        pcb_as_str(actual, str, sizeof(str)); 
        mvwprintw(process, fila, 2, str);
        actual = actual->sig;
        fila++;
    }
    if (fila < 18) {
        clear_window_part(process, fila, 2, 12 - fila, 76); 
    }

    wrefresh(process); 
}

int 
main(void) {
    cpu = cpu_new();
    initscr();
    noecho();
    cbreak();
    curs_set(1);

    msg_init();
    process_init();

    struct prompt prompt;
    reg = newwin(7, 80, 10, 0);
    box(reg, 0, 0);
    wrefresh(reg);
    prompt.win = newwin(7, 80, 17, 0);
    box(prompt.win, 0, 0);
    nodelay(prompt.win, TRUE); 
    keypad(prompt.win, TRUE);
    prompt.hist.current = 0;
    prompt.hist.size = 0;
    prompt.hist_index = 0;

    box(prompt.win, 0, 0);
    mvwprintw(prompt.win, 0, 35, "|Prompt|");
    regwin_update();
    // Inicializar las listas de procesos
    Lista listos, ejecucion, terminados;
    inicializarListas(&listos, &ejecucion, &terminados);

    // Inicializar el quantum
    int quantum = 0;

    while (true) {
        enum prompt_status status = prompt_update(&prompt);
        if (status == PROMPT_STATUS_INSTRUCTION_DECODED) {
            eval(prompt.decoded_inst, &listos);
            free(prompt.decoded_inst);
            prompt.decoded_inst = NULL;
        }

        // Ejecutar procesos
        ejecutarProcesos(&listos, &ejecucion, &terminados, &quantum);

        // Mostrar el estado en la ventana de list
        process_update(&ejecucion, &listos, &terminados);

        wrefresh(prompt.win);
        wrefresh(reg);
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
    liberarLista(&listos);
    liberarLista(&ejecucion);
    liberarLista(&terminados);

    endwin();
    return 0;
}

//DETALLES
//SE ESCRIBE MAL MIENTRAS SE EJECUTA LA PARTE DE LISTAS
//NO AUMENTA EL VALOR DEL PID
//NO SE REFRESCA CUANDO SE LLENA LA LISTA DE TERMINADOS, NO MUESTRA LOS MAS NUEVOS
//SE EJECUTAN DEMASIADO RAPIDO LOS PROCESOS
