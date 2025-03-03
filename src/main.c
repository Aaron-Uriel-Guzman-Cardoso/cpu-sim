#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "../include/cpu.h"

#define MAX_CMD_CHARS 50

struct cmd {
    char name[MAX_CMD_CHARS];
    char arg1[MAX_CMD_CHARS];
    char arg2[MAX_CMD_CHARS];
};

struct pcb *pcb;


/*
 * Imprime un prompt y obtiene un comando ingresado por el usuario.
 */
struct cmd *
prompt(void)
{
    struct cmd *cmd = malloc(sizeof(*cmd));
    if (!cmd) { return NULL; }
    const size_t MAX_BUF_SIZE = 120;
    char buf[] = "LOAD PROG", c;
    sscanf(buf, "%s %s %s", cmd->name, cmd->arg1, cmd->arg2);
    for(int i = 0; cmd->name[i] != '\0'; i += 1) {
        cmd->name[i] = toupper(cmd->name[i]);
    }
    return cmd;
}

//Funcion para imprimir mensajes en la ventana de messages
void messages_win(WINDOW *messages, const char *mensaje) {
    box(messages, 0, 0); 

    mvwprintw(messages, 0, 35, "|Messages|");

    mvwprintw(messages, 1, 2, "%s", mensaje);

    wrefresh(messages);
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
        messages_win(messages, "Error: Comando nulo.");
        return -1;
    }

    if (strncmp(cmd->name, "EXIT", 4) == 0 || strncmp(cmd->name, "SALIR", 5) == 0) {
        messages_win(messages, "Saliendo del programa...\n");
        wrefresh(messages);
        exit(0);
    } 
    else if (strncmp(cmd->name, "LOAD", 4) == 0) {
        if (cmd->arg1[0] == '\0') {
            messages_win(messages, "Error: Falta el nombre del archivo\n");
            return 1;
        } else {
            char mensaje[300];
            snprintf(mensaje, sizeof(mensaje), "Cargando archivo: %s\n", cmd->arg1);
            messages_win(messages, mensaje);

            FILE *file = fopen(cmd->arg1, "r");
            if (file) {
                messages_win(messages, "Archivo cargado con éxito.\n");
                fclose(file);
            } else {
                snprintf(mensaje, sizeof(mensaje), "Error: No se pudo abrir el archivo %s\n", cmd->arg1);
                messages_win(messages, mensaje);
            }
        }
    }

    return 0;
}

int
main(void)
{
    struct cpu *cpu = cpu_new();
    cpu_load_instfile(cpu, "P_ENDMORE");
    for (;;) {
        if (cpu_next_cycle(cpu) == 1) {
            printf("Error: No se pudo decodificar la instrucción\n");
            break;
        }
    }
    return 0;
}
