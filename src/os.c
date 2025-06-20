#include <os.h>
#include <ram.h>
#include <msg.h>
#include <string.h>
#include <user_control.h>
#include <assert.h>
#include <swap.h>
#include <tms_disp.h>
#include <swap_disp.h>
#include <swap.h>
#include <tui.h>

#include <stdlib.h> /* No deberíamos de incluirla por aquí xd */
#include <stdio.h>

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
        uint32_t progs; /*< Programas cargados en el sistema operativo */
        uint32_t procs; /*< Procesos en ejecución */
        uint32_t users; /*< Usuarios activos con procesos en ejecución */
    } stats;
    struct {
        Lista *ready;    /*< Listos para ejecutar */
        PCB *running;    /*< Único proceso en ejecución */
        Lista *finished; /*< Procesos terminados */
        Lista *new;      /*< En espera de entrar a swap*/
    } procs; /*< Colas de procesos en sus distintos estados */
    struct {
        enum proc_state kind; /*< Tipo de iteración */
        enum proc_state curr_list; /*< Solo usado para iterar con PROC_ANY */
        PCB *current; /*< Actual a regresar */
    } procs_iter; /*< Ayuda para iterar sobre los procesos */
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
    os.procs.ready = crearLista();
    os.procs.finished = crearLista();
    os.procs.new = crearLista();
    os.procs.running = NULL;
    if (os.procs.ready == NULL || os.procs.finished == NULL || os.procs.new == NULL) {
        assert(0 && "Error al crear las listas de procesos");
        return -1;
    }

    uc_init();
    
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
    return (os.procs.running && os.procs.running)
        ? os.procs.running->PID
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
            strcmp(current->program->filename, pcb->program->filename) == 0) {
            return current; // Retorna el primer hermano encontrado
        }
        current = current->sig;
    }
    
    current = os.procs.running;
    while (current) {
        if (current != pcb && current->UID == pcb->UID && 
            strcmp(current->program->filename, pcb->program->filename) == 0) {
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
        return NULL;
    }
    PCB *proceso = listaBuscarPID(os.procs.ready, pid);
    if (proceso) {
        return proceso;
    }

    if (os.procs.running && os.procs.running->PID == pid) {
        return os.procs.running;
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
    if (!os.procs.running && os.procs.ready->inicio) {
        os.procs.running = listaExtraePrioridad(os.procs.ready);
        if (os.procs.running == NULL) {
            msg_log(LOG_LEVEL_INFO, "No hay procesos listos para ejecutar.\n");
            return 0; // No hay procesos para ejecutar
        }
        os.quantum = 0; // Reiniciar el quantum

        // Verificar si el proceso ya tiene contexto previo (fue interrumpido por quantum)
        if (os.procs.running->context.regs[REG_PC] > 1) {
            // El proceso ya se ejecutó antes - restaurar su contexto
            cpu_reset();
            if (cpu_load_from_context(os.procs.running->context) != 0) {
                char logstr[350];
                snprintf(logstr, sizeof(logstr), "Error al cargar el contexto del proceso %d\n", os.procs.running->PID);
                msg_log(LOG_LEVEL_ERROR, logstr);
            } else {
                msg_log(LOG_LEVEL_INFO, "Proceso restaurado desde contexto guardado.");
            }
        }
        cpu_enable(); /* Habilitamos CPU para ejecutar instrucciones, ya
                            estando lista. */
        return_status = 1;
        tui_processes_update();
    }

    /**
     * Ejecutamos el proceso en ejecución, si es que hay uno.
     */
    if (os.procs.running) {
        PCB *proceso = os.procs.running;

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
                // Proceso finalizado, liberamos su espacio 
                os_kill_proc(proceso->PID);

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
                        if (swap_load_prog(candidato) == 0) {
                            // Proceso cargado correctamente, mover a `os.procs.ready`
                            listaInsertarFinal(os.procs.ready, candidato);
                            char buffer[300];
                            snprintf(buffer, sizeof(buffer), "Proceso %d cargado desde SWAP: %s\n",
                                     candidato->PID, candidato->program->filename);
                            msg_log(LOG_LEVEL_INFO, buffer);
                            listaExtraeInicio(os.procs.new);
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
                tui_processes_update();
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
            PCB *running = os.procs.running;
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
            tui_processes_update();
            return 0; // Salir para no procesar más eventos en este ciclo
        }
    }
}

/**
 * \brief Prepara la función `os_procs_iter_next` para iterar sobre un conjunto
 *        específico basándose en su estado.
 */
void
os_procs_iter_init(enum proc_state kind)
{
    os.procs_iter.kind = kind;
    switch (kind) {
    case PROC_ANY:
        os.procs_iter.current = os.procs.running;
        os.procs_iter.curr_list = PROC_RUNNING;
    case PROC_READY:
        os.procs_iter.current = os.procs.ready->inicio;
        break;
    case PROC_RUNNING: /*< No hay una lista de procesos en ejecución */
        os.procs_iter.current = os.procs.running;
        break;
    case PROC_FINISHED:
        os.procs_iter.current = os.procs.finished->inicio;
        break;
    case PROC_NEW:
        os.procs_iter.current = os.procs.new->inicio;
        break;
    default:
        break;
    }
}

/**
 * \brief Itera sobre un conjunto de procesos especificado en el sistema
 *        operativo
 * 
 * \return Información de control editable sobre el proceso actual en el
 *         iterador.
 */
PCB *
os_procs_iter_next(void)
{
    const PCB *returned_proc = os.procs_iter.current;
    if (returned_proc) {
        os.procs_iter.current = returned_proc->sig;
    }
    else if (os.procs_iter.kind == PROC_ANY) {
        /**
         * Si acabamos una lista y estamos en el modo para iterar en todos los
         * procesos, entonces cambiamos a la siguiente lista de procesos en el
         * orden PROC_RUNNING -> PROC_READY -> PROC_FINISHED -> PROC_NEW.
         */
        switch (os.procs_iter.curr_list) {
        case PROC_RUNNING:
            os.procs_iter.current = os.procs.ready->inicio;
            os.procs_iter.curr_list = PROC_READY;
            break;
        case PROC_READY:
            os.procs_iter.current = os.procs.finished->inicio;
            os.procs_iter.curr_list = PROC_FINISHED;
            break;
        case PROC_FINISHED:
            os.procs_iter.current = os.procs.new->inicio;
            os.procs_iter.curr_list = PROC_NEW;
            break;
        }
    }
    
    return returned_proc;
}

/**
 * \brief Obtiene el número de programas cargados en memoria
 *
 * No confundir con el número de procesos en ejecución, ya que un solo programa
 * puede tener múltiples instancias (procesos) en ejecución, y estos procesos a
 * su vez podrían compartir memoria.
 *
 * \return Número de programas cargados en memoria.
 */
uint32_t
os_num_progs(void)
{
    return os.stats.progs;
}

/**
 * \brief Obtiene el número de procesos en ejecución.
 *
 * \return Número de procesos en ejecución.
 */
uint32_t
os_num_procs(void)
{
    return os.stats.procs;
}

/**
 * \brief Obtiene el número de usuarios activos en el sistema.
 *
 * Un usuario se considera activo si tiene al menos un proceso en ejecución.
 *
 * \return Número de usuarios activos.
 */
uint32_t
os_num_users(void)
{
    return os.stats.users;
}

/**
 * \brief Crea un nuevo proceso a partir del archivo especificado.
 * 
 * Esta función se encargará de todo el **proceso** de creación para un nuevo
 * proceso: cargar el archivo, convertirlo en un programa, cargarlo en swap,
 * asignar un PID, crear el usuario a partir del uid en caso de ser necesario y
 * preparar toda su información de control para su manejo y posterior ejecución.
 */
uint32_t
os_new_proc(char *filename, uint8_t uid)
{
    if (!filename || strlen(filename) == 0) {
        msg_log(LOG_LEVEL_ERROR, "Error: Nombre de archivo inválido.\n");
        return 1; // Error por nombre de archivo inválido
    }
    FILE *progfile = fopen(filename, "r");
    if (!progfile) {
        msg_log(LOG_LEVEL_ERROR, "Error: El archivo no existe o no se puede abrir.\n");
        return 2; // Error por archivo no encontrado
    }
    fclose(progfile);

    /**
     * Hmmmm, siento que por aquí hay algo que mejorar con la dinámica
     * programa y PCB, pero por ahora lo dejamos así.
     */
    struct prog *new_prog = prog_new(filename);

    // Crear un nuevo proceso y lo agrega a la lista
    PCB *nuevo_proceso = listaCreaNodo(new_prog, uid);

    switch(swap_load_prog(nuevo_proceso)) {
    case 1:
        listaInsertarFinal(os.procs.new, nuevo_proceso);
        msg_log(LOG_LEVEL_INFO, "Proceso en espera de carga en SWAP.\n");
        break;
    case -1:
        msg_log(LOG_LEVEL_ERROR, "Error: El programa es demasiado grande para la memoria SWAP.\n");
        free(nuevo_proceso);
        return 1;
        break;
    case 0:
        if (nuevo_proceso != NULL && nuevo_proceso->program != NULL) {
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

            listaInsertarFinal(os.procs.ready, nuevo_proceso);
            msg_log(LOG_LEVEL_INFO, "Proceso agregado a la lista de Listos.\n");
            tms_disp_update();  
            swap_disp_update();
        }
        break;
    case 2:
        listaInsertarFinal(os.procs.ready, nuevo_proceso);
        msg_log(LOG_LEVEL_INFO, "Proceso hermano guardado.\n");
        break;
    }
    tui_processes_update();

}

/**
 * \brief **Asesina** a uno de los procesos en el sistema operativo
 * 
 * En caso de encontrarse al proceso se le elimina de la lista de procesos
 * en ejecución, se añade a los procesos terminados y se libera su
 * memoria asociada cuando ya nadie más la esté usando.
 * 
 * \param pid Identificador del proceso a eliminar.
 * 
 * \return 0 si el proceso fue eliminado correctamente, 1 si no se encontró
 */
int32_t
os_kill_proc(uint16_t pid)
{
    PCB *proceso = NULL;
    /**
     * Este es el caso especial cuando el proceso en ejecución es el que será
     * terminado, esto puede ocurrir cuando un programa llega a su final
     * naturalmente.
     */
    bool terminating_running = os.procs.running && os.procs.running->PID == pid;
    if (terminating_running) {
        proceso = os.procs.running;
        os.procs.running = NULL;
        proceso->context = cpu_dump_context();
        proceso->KCPU += os.quantum * IncCPU;
    } else {
        proceso = listaExtraePID(os.procs.ready, pid);
    }
    if (proceso == NULL) {
        char mensaje[300];
        snprintf(mensaje, sizeof(mensaje), "Error: No se encontró el proceso con PID %d.\n", pid);
        msg_log(LOG_LEVEL_ERROR, mensaje);
        return 1;
    }

    /**
     * TODO: hacer lo mismo pero para RAM
     */
    swap_free_frames(proceso);

    proceso->context = cpu_dump_context();
    listaInsertarFinal(os.procs.finished, proceso);

    User *user = uc_get_user(proceso->UID);
    if (user) {
        if (terminating_running) {
            user->KCPUxU += os.quantum * IncCPU;
            /**
             * ¿Es bueno reiniciar el quantum aquí o en os_process_handler?
             */
            os.quantum = 0;
        }
        assert(user->process_counter > 0);
        user->process_counter -= 1;
        if (user->process_counter == 0) {
            update_user_stats(user->uid, user->KCPUxU);
            uc_dealloc_user(user->uid);
        }
    }
    tms_disp_update();
    tui_processes_update();
}