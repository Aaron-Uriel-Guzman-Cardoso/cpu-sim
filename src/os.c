#include <os.h>
#include <ram.h>
#include <msg.h>
#include <string.h>
#include <user_control.h>
#include <assert.h>
#include <swap.h>
#include <tms_disp.h>
#include <swap_disp.h>

#include <stdlib.h> /* No deberíamos de incluirla por aquí xd */

const int32_t MAX_QUANTUM = 4;

const int32_t PBase = PBASE;
float IncCPU = 60/MAX_QUANTUM;
double W = 0.0; // Peso de la CPU

/** 
 * \brief Estructura que representa a la tabla de marcos de memoria de
 *        acceso aleatorio
 */
struct tmm {
    /* ¿Qué opinan de una estructura anónima xd? */
    struct { uint16_t pid; bool touched; } rows[RAM_NUM_FRAMES]; /*< Índice es el marco */
    uint16_t clock_pos; /*< Índice usado en algoritmo del reloj */
} tmm;

/**
 * \brief Permite marcar como leído un marco de la RAM
 * 
 * Esto usualmente solo debería de ocurrir cuando estés usando una función
 * que lea algo en RAM a nivel de sistema operativo, como por ejemplo en
 * `mmu_get_inst()`.
 * Nota: podríamos hacer que `ram_read()` y similarews llamen a esta función
 *       sin embargo, esto no es tan ralista en términos de abstracción.
 */
int32_t
tmm_touch(uint16_t frame)
{
    if (frame >= RAM_NUM_FRAMES) {
        return -1;
    }
    tmm.rows[frame].touched = 1;
    return 0;
}

struct os {
    uint32_t quantum; /*< Quantum para round-robin */
    struct {
        Lista *ready;    /*< Listos para ejecutar */
        Lista *running;  /*< Único proceso en ejecución */
        Lista *finished; /*< Procesos terminados */
        Lista *new;      /*< En espera de entrar a swap*/
    } procs; /*< Colas de procesos en sus distintos estados */
} os;

/**
 * \brief Realiza inicializaciones necesarias para hacer funcionar el sistema
 *        operativo simulado
 *
 * Actualmente solo inicializa la listas de procesos usadas para manejar los
 * estados de ejecución.
 *
 * \return Retorna 0 si la ventana se inicializó correctamente.
 */
int32_t
os_init(void)
{
    os.procs.ready = malloc(sizeof(*os.procs.ready));
    os.procs.running = malloc(sizeof(*os.procs.running));
    os.procs.finished = malloc(sizeof(*os.procs.finished));
    os.procs.new = malloc(sizeof(*os.procs.new));
    if (os.procs.ready == NULL || os.procs.running == NULL || os.procs.finished == NULL) {
        msg_log(LOG_LEVEL_ERROR, "Error al inicializar las listas de procesos.\n");
        return -1;
    }
    crearLista(os.procs.ready);
    crearLista(os.procs.running);
    crearLista(os.procs.finished);
    crearLista(os.procs.new);
    
   return 0;
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
    return (os.procs.running && os.procs.running->inicio)
        ? os.procs.running->inicio->PID
        : 0;
}

/**
 * \brief Busca un hermano (sibling) de un proceso dado.
 *
 * Un hermano es un proceso que pertenece al mismo usuario (UID) y ejecuta
 * el mismo archivo (fileName), pero no es el mismo proceso 
 * (diferente dirección de PCB). La función busca primero en la lista de
 * procesos os.procs.ready y luego en la lista de procesos en ejecución.
 *
 * \param pcb Puntero al proceso (PCB) del cual se busca un hermano.
 * \return Puntero al primer hermano encontrado, o NULL si no existe.
 */
PCB *
os_find_sibling(PCB *pcb) {
    PCB *current = os.procs.ready->inicio;
    while (current) {
        if (current != pcb && current->UID == pcb->UID && 
            strcmp(current->fileName, pcb->fileName) == 0) {
            return current; // Retorna el primer hermano encontrado
        }
        current = current->sig;
    }
    
    current = os.procs.running->inicio;
    while (current) {
        if (current != pcb && current->UID == pcb->UID && 
            strcmp(current->fileName, pcb->fileName) == 0) {
            return current; // Retorna el primer hermano encontrado
        }
        current = current->sig;
    }
    
    return NULL;
}

/**
 * \brief Busca un proceso por su PID en las listas de os.procs.ready y ejecución.
 *
 * La función busca un proceso cuyo identificador (PID) coincida con el dado.
 * Primero busca en la lista de procesos os.procs.ready, luego en la de ejecución.
 *
 * \param pid Identificador del proceso a buscar.
 * \return Puntero al PCB del proceso si se encuentra, o NULL si no existe.
 */
struct PCB *
os_get_proc(uint16_t pid)
{
    if (pid == 0) {
        return NULL; // No hay proceso con PID 0
    }
    PCB *proceso = listaBuscarPID(os.procs.ready, pid);
    if (proceso) {
        return proceso;
    }

    proceso = listaBuscarPID(os.procs.running, pid);
    if (proceso) {
        return proceso;
    }
    return NULL; // No se encontró el proceso
}

/**
 * \brief Maneja los fallos de páginas identificados por el MMU
 * 
 * Esta función asume que se ha determinado un fallo de página en la RAM por
 * parte del MMU, aquí es donde se implementa el algoritmo del reloj 
 * (o de segunda oportunidad) para buscar el reemplazo de un solo marco para
 * satisfacer la necesidad de una página.
 * 
 * \param proc El proceso del cual se ha determinado que no tiene una pagina de
 *             swap cargada en RAM.
 * \param page El índice de página en la TMP del proceso en cuestión que no se
 *             encuentra cargada cargada en RAM y se va a utilizar.
 */
void
os_page_fault_handler(PCB *proc, uint16_t page)
{
    for (;
         tmm.rows[tmm.clock_pos].touched != 0;
         tmm.clock_pos = ((tmm.clock_pos + 1) % RAM_NUM_FRAMES))
    {
        if (tmm.rows[tmm.clock_pos].touched) {
            tmm.rows[tmm.clock_pos].touched = 0;
        }
    }
    uint16_t target_frame = tmm.clock_pos;
    word_t frame2load[RAM_FRAME_SIZE];
    swap_get_page(frame2load, proc->tmp_rows[page].on_swap);
    
    ram_write(ram_frame_to_addr(target_frame), frame2load, sizeof(frame2load));
    proc->tmp_rows[page].on_ram = target_frame;
}

/**
 * \brief Maneja la ejecución de todos los procesos del sistema operativo
 *
 * Esta función se encarga de gestionar la ejecución de los procesos en las 
 * distintas colas que para representación de los distintos estados de los
 * procesos en el sistema operativo (os.procs.new, os.procs.ready, ejecución y os.procs.finished).
 * Esta función tiene dos schedulers implementados de forma no fácilmente
 * reemplazable: round-robin, y fair-share scheduler.
 *
 * Si no hay un proceso en ejecución y hay procesos en la lista de os.procs.ready, mueve el primer
 * proceso de la lista de os.procs.ready a la lista de ejecución. Luego, ejecuta las instrucciones
 * del proceso en ejecución y maneja los eventos generados por la CPU.
 *
 * \return 0 si no ha ocurrido un cambio de contexto, 1 si se ha cambiado el
 *         contexto a un nuevo proceso, 2 si ha habido un problema con las listas.
 */
int32_t
os_process_handler(void)
{
    int32_t return_status = 0;
    /**
     * Empezar a *ejecutar* el proceso que está más *listo* (mayor prioridad)
     */
    if (os.procs.running->inicio == NULL && os.procs.ready->inicio != NULL) {
        PCB *proceso = listaExtraePrioridad(os.procs.ready);

        listaInsertarFinal(os.procs.running, proceso);
        os.quantum = 0; // Reiniciar el quantum

        // Verificar si el proceso ya tiene contexto previo (fue interrumpido por quantum)
        if (proceso->context.regs[REG_PC] > 1) {
            // El proceso ya se ejecutó antes - restaurar su contexto
            cpu_reset();
            if (cpu_load_from_context(proceso->context) != 0) {
                char logstr[350];
                snprintf(logstr, sizeof(logstr), "Error al cargar el contexto del proceso %d\n", proceso->PID);
                msg_log(LOG_LEVEL_ERROR, logstr);
            } else {
                msg_log(LOG_LEVEL_INFO, "Proceso restaurado desde contexto guardado.");
            }
        }
        cpu_enable(); /* Habilitamos CPU para ejecutar instrucciones, ya
                            estando lista. */
        return_status = 1;
    }

    /**
     * Ejecutamos el proceso en ejecución, si es que hay uno.
     */
    if (os.procs.running->inicio != NULL) {
        PCB *proceso = os.procs.running->inicio;

        /* 
         * Cuando la CPU no ejecutó ninguna instrucción desde la última vez
         * que consultamos, no tenemos nada que hacer.
         */
        if (!cpu_sync()) {
            return 0;
        }

        // Manejar eventos generados por la CPU
        enum cpu_event event;
        while ((event = cpu_poll_event()) != CPU_NONE) {
            if (event == CPU_HALT) {
                // Proceso finalizado, moverlo a os.procs.finished
                PCB *finished = listaExtraeInicio(os.procs.running);
                finished->KCPU += os.quantum * IncCPU;

                // Actualizar estadísticas del usuario
                User *user = uc_get_user(finished->UID);
                if (user) {
                    user->KCPUxU += os.quantum * IncCPU;
                    assert(user->process_counter != 0);
                    user->process_counter -= 1;
                    if (user->process_counter == 0) {
                        update_user_stats(user->uid, user->KCPUxU);
                        uc_dealloc_user(user->uid);
                    }
                }

                if (finished == NULL) {
                    msg_log(LOG_LEVEL_ERROR, "Error: No se pudo extraer el proceso de ejecución.\n");
                    return 2;
                }
                finished->context = cpu_dump_context();

                listaInsertarFinal(os.procs.finished, finished);
                swap_free_frames(finished);
                tms_disp_update();
                os.quantum = 0;

                /*
                 * Al eliminar un proceso del swap, intentamos cargar uno nuevo
                 * de entre todos en el swap
                 * Nota: Esto puede hacer que procesos muy grandes nunca entren
                 *       a la memoria swap porque está llena de procesos
                 *       pequeños, es necesario aclarar como hacer esto con el
                 *       profesor.
                 */
                if (os.procs.new->inicio) {
                    PCB *candidato = os.procs.new->inicio;
                    do {
                        if (swap_load_program1(candidato, candidato->fileName) == 0) {
                            // Proceso cargado correctamente, mover a `os.procs.ready`
                            listaInsertarFinal(os.procs.ready, candidato);
                            char buffer[300];
                            snprintf(buffer, sizeof(buffer), "Proceso %d cargado desde SWAP: %s\n",
                                     candidato->PID, candidato->fileName);
                            msg_log(LOG_LEVEL_INFO, buffer);
                            listaExtraeInicio(os.procs.new);
                            process_disp_update();
                            tms_disp_update();
                            swap_disp_update();
                            break;
                        }
                    } while ((candidato = candidato->sig));

                    User *user;
                    if (!uc_user_exists(candidato->UID)) {
                        user = crearUsuario(candidato->UID);
                        user->process_counter = 0;
                        uc_alloc_user(user);
                    } else {
                        user = uc_get_user(candidato->UID);
                    }
                    user->process_counter += 1;
                    
                }

                cpu_reset();
                process_disp_update();

                // Mover procesos de Nuevos a Listos
                while (os.procs.new->inicio != NULL) { 
                PCB *proceso_nuevo = os.procs.new->inicio; 
                if (swap_load_program1(proceso_nuevo, proceso_nuevo->fileName) == 0) {
                    proceso_nuevo = listaExtraeInicio(os.procs.new); 
                    listaInsertarFinal(os.procs.ready, proceso_nuevo); 

                    User *user_new = uc_get_user(proceso_nuevo->UID);
                    if (user_new) {
                       user_new->process_counter += 1;
                    }
                    
                    msg_log(LOG_LEVEL_INFO, "Proceso movido de Nuevos a Listos.\n");
                    process_disp_update();
                    tms_disp_update();
                } else {
                    msg_log(LOG_LEVEL_WARN, "No hay espacio en SWAP para el proceso. Permanece en Nuevos.\n");
                    break; 
                }
                }

                return 0; // Salir después de manejar el evento de terminación
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

        /**
         * ¿Ya superamos el quantum máximo?
         * Si es así, guardamos el contexto del proceso en ejecución y lo movemos
         * de vuelta a os.procs.ready.
         */
        if (++os.quantum >= MAX_QUANTUM) {
            PCB *running = listaExtraeInicio(os.procs.running);
            if (!running) {
                msg_log(LOG_LEVEL_ERROR, "Error: No se pudo extraer el proceso de ejecución.\n");
                return 2;
            }
            running->context = cpu_dump_context();
            running->KCPU += os.quantum * IncCPU;

            // Actualizar estadísticas del usuario
            User *user = uc_get_user(running->UID);
            if (user) {
                user->KCPUxU += os.quantum * IncCPU;
                update_user_stats(user->uid, user->KCPUxU);
            }

            listaInsertarFinal(os.procs.ready, running);

            // Recalcular prioridades de los procesos en la lista de os.procs.ready
            PCB *current_process = os.procs.ready->inicio;
            do {
                current_process->KCPU /= 2;
                User *user = uc_get_user(current_process->UID);
                if (user) {
                    user->KCPUxU /= 2;
                    current_process->P = 
                        PBase + current_process->KCPU / 2 +
                        (user->KCPUxU / (4 * uc_get_weight()));
                }
            } while ((current_process = current_process->sig));

            os.quantum = 0;
            cpu_reset();
            process_disp_update();
            return 0; // Salir para no procesar más eventos en este ciclo
        }

        
    }
}