#include <curses.h>
#include <msg.h>
#include <ncurses_utilities.h>
#include <stdint.h>

WINDOW *msg;
WINDOW *list;

/*
 * Inicializa la ventana de mensajes para que esta pueda pueda ser utilizada.
 * Esta función tendrá que ser llamada después de que ncurses esté
 * inicializada.
 */
int32_t
msg_init(void)
{
    /*
     * Las coordenadas están hardcoded, no se me ocurre una buena forma de
     * hacer automático el cálculo de las dimensiones de las ventanas.
     */
    msg = newwin(10, 80, 0, 0);
    box(msg, 0, 0);
    wrefresh(msg);
    return 0;
}

int32_t
list_init(void)
{
    list = newwin(10, 80, 0, 85);
    box(list, 0, 0);
    wrefresh(list);
    return 0;
}

/*
 * Agrega el mensaje `msg` a la ventana correspondiente.
 */
int32_t
msg_log(enum log_level level, const char *str)
{
    /*
     * Por ahora solo limpiamos la pantalla e imprimimos pero en un futuro
     * deberíamos de ser capaces de mostrar un historial completo de mensajes.
     */
    clear_window_part(msg, 1, 1, 8, 78);
    /*
     * Queda pendiente también imprimir con colores los mensajes de info,
     * advertencia y error.
     */
    mvwprintw(msg, 1, 2, "%s", str);
    wrefresh(msg);
    return 0;
}
