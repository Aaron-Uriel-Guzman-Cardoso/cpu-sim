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
struct cpu *cpu; /* Tendremos una única CPU en el simulador */
struct timespec cpu_period = { 2, 0 }; /* Frecuencia de ejecucón de la cpu */ 

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




/*
 *Prototipo: int32_t eval(struct instruction *cmd, WINDOW *messages, PCB *pcb) {
 *Propósito: Esta función evalúa e interpreta un comando ingresado por el usuario
 *y ejecuta la acción correspondiente, como salir del programa o cargar un archivo en el PCB.
 *Entradas: cmd(Puntero con la instruccion leida), messages(Puntero a la ventana messages) y pcb 
 *(Puntero a la estructura PCB)
 *Salidas: Retorna 0 en caso de exito, 1 en caso de error y -1 en caso de un comando no reconocido.
 *Descripción: Evalúa el comando ingresado y realiza la acción correspondiente. Si el comando es "EXIT" o "SALIR",
 * cierra el programa. Si el comando es "LOAD", intenta cargar el archivo especificado. Si el comando es nulo o no
 * reconocido, muestra un mensaje de error.
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

            // Cargar el archivo en la CPU
            cpu_reset(cpu);
            int32_t result = cpu_load_insts_from_file(cpu, cmd->arg1);
            
            if (result == 0) {
                msg_log(LOG_LEVEL_INFO, "Archivo cargado con éxito.\n");

                // Crear un nuevo proceso y lo agrega a la lista
                PCB *nuevo_proceso = listaCreaNodo(0, 0, 0, 0, 0, "", cmd->arg1);
                if (nuevo_proceso != NULL && nuevo_proceso->programa != NULL) {
                    listaInsertarFinal(listos, nuevo_proceso);
                    msg_log(LOG_LEVEL_INFO, "Proceso agregado a la lista de Listos.\n");
                } else {
                    msg_log(LOG_LEVEL_ERROR, "Error al crear el proceso.\n");
                    if (nuevo_proceso != NULL) {
                        free(nuevo_proceso); 
                    }
                }
            } else {
                snprintf(mensaje, sizeof(mensaje), "Error al cargar archivo %s (código: %d)\n", 
                cmd->arg1, result);
                msg_log(LOG_LEVEL_ERROR, mensaje);
            }
        }
    }
    else { 
        msg_log(LOG_LEVEL_ERROR, "Comando nulo.");
        return -1;
    }

    return 0;
}

void inicializarListas(Lista *listos, Lista *ejecucion, Lista *terminados) {
    crearLista(listos);
    crearLista(ejecucion);
    crearLista(terminados);
}

void cargarProceso(Lista *listos, const char *fileName) {
    PCB *proceso = listaCreaNodo(0, 0, 0, 0, 0, "", fileName);
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

void ejecutarProcesos(Lista *listos, Lista *ejecucion, Lista *terminados, int *quantum, const int MAXQUANTUM) {
    // Si no hay proceso en ejecución y hay procesos en Listos, mover el primero a Ejecución
    if (ejecucion->inicio == NULL && listos->inicio != NULL) {
        PCB *proceso = listaExtraeInicio(listos);
        listaInsertarFinal(ejecucion, proceso);
        *quantum = 0; // Reiniciar el quantum
    }

    // Si hay un proceso en ejecución, ejecutar una instrucción
    if (ejecucion->inicio != NULL) {
        PCB *proceso = ejecucion->inicio;

        // Leer la siguiente instrucción del archivo
        char instruccion[100];
        if (fgets(instruccion, sizeof(instruccion), proceso->programa) != NULL) {
            // Actualizar el registro IR con la instrucción leída
            strncpy(proceso->IR, instruccion, sizeof(proceso->IR) - 1);
            proceso->IR[sizeof(proceso->IR) - 1] = '\0'; 

            // Incrementar el contador de programa (PC)
            proceso->PC++;

            // Incrementar el quantum
            (*quantum)++;

            // Si el quantum alcanzó el máximo, mover el proceso a Listos
            if (*quantum >= MAXQUANTUM) {
                PCB *proceso = listaExtraeInicio(ejecucion);
                listaInsertarFinal(listos, proceso);
                *quantum = 0; // Reiniciar el quantum
            }
        } else {
            // Si no hay más instrucciones, mover el proceso a Terminados
            PCB *proceso = listaExtraeInicio(ejecucion);
            listaInsertarFinal(terminados, proceso);
            fclose(proceso->programa); // Cerrar el archivo del proceso

            char mensaje[300];
            snprintf(mensaje, sizeof(mensaje), "Proceso terminado: PID %d\n", proceso->PID);
            msg_log(LOG_LEVEL_INFO, mensaje);
            
        }
    }
}

void mostrarEstado(WINDOW *list, Lista *ejecucion, Lista *listos, Lista *terminados) {
    clear_window_part(list, 1, 2, 11, 76);

    // Mostrar el proceso en ejecución
    mvwprintw(list, 1, 2, "......PROCESADOR......");
    if (ejecucion->inicio != NULL) {
        PCB *proceso = ejecucion->inicio;
        mvwprintw(list, 2, 2, "AX:[%d] PC:[%d] SIG:[%d]", proceso->AX, proceso->PC, proceso->sig ? proceso->sig->PID : -1);
        mvwprintw(list, 3, 2, "BX:[%d] IR:[%s]", proceso->BX, proceso->IR);
        mvwprintw(list, 4, 2, "CX:[%d] PID:[%d]", proceso->CX, proceso->PID);
        mvwprintw(list, 5, 2, "DX:[%d] NAME:[%s]", proceso->DX, proceso->fileName);
    } else {
        clear_window_part(list, 2, 2, 4, 76); 
    }

    // Mostrar la lista de procesos listos
    mvwprintw(list, 7, 2, "......LISTA......");
    PCB *actual = listos->inicio;
    int fila = 8;
    while (actual != NULL && fila < 10) { 
        mvwprintw(list, fila, 2, "PID:[%d] FILE:[%s]", actual->PID, actual->fileName);
        actual = actual->sig;
        fila++;
    }
    if (fila < 10) {
        clear_window_part(list, fila, 2, 10 - fila, 76); 
    }

    // Mostrar la lista de procesos terminados
    mvwprintw(list, 9, 2, "......TERMINADOS......");
    actual = terminados->inicio;
    fila = 10;
    while (actual != NULL && fila < 18) { 
        mvwprintw(list, fila, 2, "PID:[%d] FILE:[%s]", actual->PID, actual->fileName);
        actual = actual->sig;
        fila++;
    }
    if (fila < 18) {
        clear_window_part(list, fila, 2, 12 - fila, 76); 
    }

    wrefresh(list); 
}

int 
main(void) {
    cpu = cpu_new();
    initscr();
    noecho();
    cbreak();
    curs_set(1);

    msg_init();
    list_init();

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

    // DECLARACIÓN DE LA VENTANA DE LISTA
    WINDOW *list = newwin(20, 80, 0, 85);
    box(list, 0, 0);
    wrefresh(list);

    box(prompt.win, 0, 0);
    mvwprintw(prompt.win, 0, 35, "|Prompt|");
    regwin_update();
    // Inicializar las listas de procesos
    Lista listos, ejecucion, terminados;
    inicializarListas(&listos, &ejecucion, &terminados);

    // Inicializar el quantum
    int quantum = 0;
    const int MAXQUANTUM = 5;

    while (true) {
        enum prompt_status status = prompt_update(&prompt);
        if (status == PROMPT_STATUS_INSTRUCTION_DECODED) {
            eval(prompt.decoded_inst, &listos);
            free(prompt.decoded_inst);
            prompt.decoded_inst = NULL;
        }

        // Ejecutar procesos
        ejecutarProcesos(&listos, &ejecucion, &terminados, &quantum, MAXQUANTUM);

        // Mostrar el estado en la ventana de list
        mostrarEstado(list, &ejecucion, &listos, &terminados);

        if (cpu_sync(cpu)) {
            regwin_update();
        }

        wrefresh(prompt.win);
        wrefresh(reg);
        wrefresh(list); // Actualizar la ventana de list_log
        /*
         * La lógica del bucle principal se ejecuta cada 33 ms
         * (más o menos 30 FPS o HZ).
         */
        struct timespec update_delay = {
            .tv_sec = 0,
            .tv_nsec = 33E6
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
