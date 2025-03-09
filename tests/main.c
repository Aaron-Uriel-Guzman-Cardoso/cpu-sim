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


int32_t
regwin_update(void)
{
    clear_window_part(reg, 1, 1, 5, 78);
    mvwprintw(reg, 2, 2, "AX: %d", cpu->regs[REG_AX]);
    mvwprintw(reg, 3, 2, "BX: %d", cpu->regs[REG_BX]);
    mvwprintw(reg, 4, 2, "CX: %d", cpu->regs[REG_CX]);
    mvwprintw(reg, 5, 2, "freq: %g Hz", 1.0/cpu_period.tv_sec);
    mvwprintw(reg, 2, 35, "DX: %d", cpu->regs[REG_DX]);
    mvwprintw(reg, 3, 35, "PC: %d", cpu->regs[REG_PC]);
    char current_inst[50];
    inst_to_str((struct inst *)&cpu->regs[REG_IR], current_inst, 50);
    mvwprintw(reg, 4, 35, "IR: %s", current_inst);
    wrefresh(reg);
    return 0;
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
eval(struct cmd *cmd, WINDOW *messages) {
    if (!cmd) { 
        msg_log( LOG_LEVEL_ERROR, "Comando nulo.");
        return -1;
    }

    if (strncmp(cmd->name, "EXIT", 4) == 0 || strncmp(cmd->name, "SALIR", 5) == 0) {
        msg_log( LOG_LEVEL_INFO, "Saliendo del programa...\n");
        wrefresh(messages);
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

            FILE *file = fopen(cmd->arg1, "r");
            if (file) {
                msg_log( LOG_LEVEL_INFO, "Archivo cargado con éxito.\n");
                fclose(file);
            } else {
                snprintf(mensaje, sizeof(mensaje), "No se pudo abrir el archivo %s\n", cmd->arg1);
                msg_log( LOG_LEVEL_ERROR, mensaje);
            }
        }
    }

    return 0;
}

struct prompt {
    char buf[120];
    size_t buflen;
    struct instruction *decoded_inst;
    WINDOW *win;
    struct cmd_history hist;
    int hist_index;
};

int
main(void) {
    cpu = cpu_new();
    initscr();
    noecho();
    cbreak();
    curs_set(1);

    msg_init();

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

    struct timespec last_cpu_execution = { 0 };
    char prog_one[] = "MoV Ax -13\n"
        "add bx 31\n"
        "inc ax\n"
        "inc ax\n"
        "dec bx\n"
        "Mul cX 3140\n"
        "aDD ax 10\n"
        "div bx ax\n"
        "end\n"
        "MOV RAX 3\n";
    cpu_load_insts_from_str(cpu, prog_one);
    while (true) {
        if (cpu->state == CPU_READY) {
            struct timespec curr, delta;
            clock_gettime(CLOCK_MONOTONIC, &curr);
            delta.tv_sec = curr.tv_sec - last_cpu_execution.tv_sec;
            delta.tv_nsec = curr.tv_nsec - last_cpu_execution.tv_nsec;
            if (delta.tv_sec > cpu_period.tv_sec) {
                cpu_next_cycle(cpu);
                last_cpu_execution = curr;
                regwin_update();
            }
        }
        /*
         * La lógica del bucle principal se ejecuta cada 33 ms
         * (más o menos 30 FPS o HZ).
         */
        usleep(33E3);
    }
    endwin();
    return 0;
}

