
#include <ncurses.h>

#include <lista.h>
#include <tui.h>
#include <os.h>
#include <user_control.h>
#include <msg.h>
#include <tms_disp.h>
#include <swap_disp.h>
#include <prompt.h>
#include <ncurses_utilities.h>

/**
 * \brief Estructura que representa la interfaz de usuario (TUI)
 *
 * Esta maneja todos el aspecto gráfico de la interfaz así como el manejo de la
 * entrada del usuario. Este "objeto" expone "métodos" para actualizar cada parte
 * individual de la interfaz, asumiendo que es el programador el que debe de saber
 * cuando se debe de actualizar algo.
 * 
 * TODO: hacer que las ventana de registros sea más bien del proceso en ejecución,
 *       ya que esta cubre datos mostrados en TMP.
 */
struct tui
{
    WINDOW *curr_proc;
    WINDOW *processes;
    WINDOW *input; /*< Usada para manejar toda entrada recibida*/
} tui;

void
tui_processes_init(void)
{
    tui.processes = newwin(44, 125, 0, 110);
    box(tui.processes, 0, 0);
    wrefresh(tui.processes);
    tui_processes_update();
}

/**
 * \brief Actualiza y muestra la información de los procesos en la TUI.
 *
 * Esta función se encarga de actualizar  el estado de los procesos en
 * ejecución, proc_ready y proc_finished.
 *
 * Limpia las secciones correspondientes de la ventana y luego imprime la
 * información actualizada de cada lista de procesos.
 */
void
tui_processes_update(void)
{
    tui.processes = newwin(44, 125, 0, 110);
    box(tui.processes, 0, 0);
    wrefresh(tui.processes);
    
    // Mostrar el conteo de archivos únicos activos (ejecución + proc_ready)
    mvwprintw(tui.processes, 1, 2, "Archivos únicos activos: %d   Usuarios activos: %d",
              os_num_progs(), os_num_users());
    mvwprintw(tui.processes, 2, 2,
              "------------------------------------------------|EJECUCION|-----------------------------------------------");

    PCB *proc = os_get_proc(os_get_curr_pid());
    if (proc)
    {
        char current_inst[50];
        inst_to_str((struct inst *)&proc->context.regs[REG_IR],
                    current_inst, sizeof(current_inst));
        User *user = uc_get_user(proc->UID);
        /**
         * ¿No debería de usarse aquí pcb_as_str()?
         * - auriel
         */
        mvwprintw(tui.processes, 3, 2,
                  "PID: %d, UID: %d, P: %d, KCPU: %.2f, KCPUxU: %.2f, File: %s, "
                  "AX: %ld, BX: %ld, CX: %ld, DX: %ld, PC: %ld, IR: %s",
                  proc->PID,
                  proc->UID,
                  proc->P,
                  proc->KCPU,
                  user_stats[proc->UID].KCPUxU,
                  proc->program->filename,
                  proc->context.regs[REG_AX],
                  proc->context.regs[REG_BX],
                  proc->context.regs[REG_CX],
                  proc->context.regs[REG_DX],
                  proc->context.regs[REG_PC],
                  current_inst);
    }
    else
    {
        clear_window_part(tui.processes, 3, 2, 1, 76);
    }

    mvwprintw(tui.processes, 4, 2,
              "--------------------------------------------------|LISTOS|--------------------------------------------------");
    os_procs_iter_init(PROC_READY);
    uint32_t fila = 5;
    while ((proc = os_procs_iter_next()))
    {
        User *user = uc_get_user(proc->UID);
        char str[200];
        pcb_as_str(proc, str, sizeof(str));
        mvwprintw(tui.processes, fila, 2, "%s", str);
        fila++;
    }

    mvwprintw(tui.processes, fila, 2,
              "--------------------------------------------------|NUEVOS|--------------------------------------------------");
    os_procs_iter_init(PROC_NEW);
    while ((proc = os_procs_iter_next()))
    {
        User *user = uc_get_user(proc->UID);
        char str[200];
        pcb_as_str(proc, str, sizeof(str));
        mvwprintw(tui.processes, fila, 2, "%s", str);
        fila++;
    }

    int fila_terminados = fila;
    clear_window_part(tui.processes, fila_terminados, 2, 18 - fila_terminados, 76);
    mvwprintw(tui.processes, fila_terminados, 2, "------------------------------------------------|TERMINADOS|-----------------------------------------------");
    while ((proc = os_procs_iter_next()))
    {
        User *user = uc_get_user(proc->UID);
        char str[200];
        float kcpuxu = get_user_stats(proc->UID);
        pcb_as_str_kcpuxu(proc, str, sizeof(str), kcpuxu);
        mvwprintw(tui.processes, fila, 2, "%s", str);
        fila++;
    }

    if (fila < 18)
    {
        clear_window_part(tui.processes, fila, 2, 18 - fila, 76);
    }
    wrefresh(tui.processes);
}


/**
 * \brief Inicializa la ventana de procesos en ejecución
 *
 * Esta función crea la ventana que muestra los procesos en ejecución, listos,
 * nuevos y terminados.
 */
void tui_curr_proc_init(void)
{
    tui.curr_proc = newwin(7, 110, 10, 0);
    box(tui.curr_proc, 0, 0);
    tui_curr_proc_update();
}

/**
 *
 * \brief Inicializa la interfaz de terminal de usuario
 *
 * Esta función crea todas las ventanas para mostrar toda la información de
 * utilidad recopilada por el simulador.
 *
 * TODO: hacer que todas las ventanas se inicialicen aquí, incluyendo swap
 *       y RAM.
 *
 */
void tui_init(void)
{
    initscr();
    raw();
    noecho();
    cbreak();
    curs_set(1);


    tui.input = newwin(1, 1, 900, 900);
    scrollok(tui.input, TRUE);
    keypad(tui.input, TRUE);
    nodelay(tui.input, TRUE);
    wmove(tui.input, 0, 0);
    wrefresh(tui.input);

    msg_init();
    prompt_init();
    swap_disp_init();
    tms_disp_init();
    tui_curr_proc_init();
    tui_processes_init();


    
}

/**
 * \brief Maneja todas las entradas del usuario
 *
 * Esta función maneja cualquier entrada dada por el usuario, dada la entrada
 * se determina si se trata de un caracter para la línea de comandos que
 * realizará una acción sobre el simulador o si se trata de una tecla de función
 * que manipula el comportamiento de la TUI.
 *
 * \return Se regresa 1 ocurrió un evento significativo, 0 si no ocurrió nada
 */
bool tui_input_handler()
{
    char in = wgetch(tui.input);
    if (in == ERR)
    {
        return false;
    }
    if (in == '\n')
    {
        prompt_enter();
    }
    else if (in == KEY_BACKSPACE || in == 127 || in == 8 ||
             in == 7)
    {
        prompt_backspace();
    }
    else if (in == KEY_F(5) || in == 13 || in == '+')
    {
        tms_disp_pg_up();
    }
    else if (in == KEY_F(6) || in == 14 || in == '-')
    {
        tms_disp_pg_dn();
    }
    else if (in == KEY_F(7) || in == 15 || in == '*')
    {
        swap_disp_pg_up();
    }
    else if (in == KEY_F(8) || in == 16 || in == '/')
    {
        swap_disp_pg_dn();
    }
    else if (in == KEY_UP || in == 3)
    {
        prompt_up();
    }
    else if (in == KEY_DOWN || in == 2)
    {
        prompt_dn();
    }
    else if (in == KEY_LEFT || in == 4)
    {
        cpu_set_freq(cpu_get_freq() / 2);
        tui_curr_proc_update();
    }
    else if (in == KEY_RIGHT || in == 5)
    {
        cpu_set_freq(cpu_get_freq() * 2);
        tui_curr_proc_update();
    }
    else
    {
        prompt_insert_char(in);
    }
}


/**
 * \brief Actualiza los valores de los registros y muestra el contenido de la TMP.
 * 
 * Esta función recurre a la información de control que el sistema operativo
 * tiene del proceso actual para la impresión.
 *
 * \return Retorna 0 si la actualización se realizó correctamente.
 */
int32_t
tui_curr_proc_update(void)
{
    clear_window_part(tui.curr_proc, 1, 1, 5, 108);
    /**
     * TODO: no recurrir directamente a la CPU para mostrar registros, requiere
     *       hacer que PCB siempre tenga los registros actualizados.
     */
    struct cpu_context context = cpu_dump_context();
    mvwprintw(tui.curr_proc, 2, 2, "AX: %ld", context.regs[REG_AX]);
    mvwprintw(tui.curr_proc, 3, 2, "BX: %ld", context.regs[REG_BX]);
    mvwprintw(tui.curr_proc, 4, 2, "CX: %ld", context.regs[REG_CX]);
    mvwprintw(tui.curr_proc, 5, 2, "freq: %g Hz", cpu_get_freq());
    mvwprintw(tui.curr_proc, 2, 18, "DX: %ld", context.regs[REG_DX]);
    mvwprintw(tui.curr_proc, 3, 18, "PC: %ld", context.regs[REG_PC]);
    char current_inst[50];
    inst_to_str((struct inst *)&context.regs[REG_IR], current_inst, 50);
    mvwprintw(tui.curr_proc, 4, 18, "IR: %s", current_inst);

    PCB *running = os_get_proc(os_get_curr_pid());
    if (running) {
        mvwprintw(tui.curr_proc, 1, 52, "Proceso PID: %d", running->PID);

        int max_lines = 7; // Espacio disponible para imprimir marcos
        int lines_used = 2;
        for (int i = 0; i < running->tmp_size && lines_used < max_lines + 8; i++) {
            mvwprintw(tui.curr_proc, lines_used, 52, "Marco %d -> SWAP Marco %03X, "
                      "RAM Marco %03X", i, running->tmp_rows[i].on_swap,
                      running->tmp_rows[i].on_ram);
            lines_used++;
        }

        if (lines_used == max_lines + 8) {
            mvwprintw(tui.curr_proc, lines_used, 52, "... (más marcos no mostrados)");
        }
    }
    else
    {
        mvwprintw(tui.curr_proc, 1, 52, "No hay proceso en ejecución.");
    }

    wrefresh(tui.curr_proc);
    return 0;
}