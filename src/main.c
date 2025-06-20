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
#include <tui.h>

#include "../include/cpu.h"

#define PBASE 60
#define MAX_USER_STATS 256

int user_stats_count = 0;

struct timespec cpu_period = { 0, 500000000 }; /* Frecuencia de ejecucón de la cpu */ 

struct timespec last_process_update = { 0 };
struct timespec process_update_period = { 0, 500000000 }; // Actualizar cada 500 ms

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
                os_new_proc(cmd->arg1, uid);
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
            char *end;
            uint64_t pid = strtol(cmd->arg1, &end, 10);
            if (pid <= 0 || pid > INT32_MAX ||
                end == cmd->arg1 || *end != '\0') {
                msg_log(LOG_LEVEL_ERROR, "PID inválido.\n");
                return 1; // Error al eliminar el proceso
            }

            char mensaje[300];
            if (!os_kill_proc(pid)) {
                snprintf(mensaje, sizeof(mensaje), "Proceso eliminado: %d\n", pid);
                msg_log(LOG_LEVEL_INFO, mensaje);
            } else {
                snprintf(mensaje, sizeof(mensaje),
                         "Error: No se encontró el proceso con PID %d.\n", pid);
                msg_log(LOG_LEVEL_ERROR, mensaje);
                return 1;
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
    ram_init();
    os_init();
    
    tui_init();

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

        os_process_handler();
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
    endwin();
    return 0;
}