#include <curses.h>
#include <msg.h>
#include <ncurses_utilities.h>
#include <stdint.h>

WINDOW *msg;
WINDOW *list;

/**
 * \brief Inicializa la ventana de mensajes.
 * \return Retorna 0 si la ventana se inicializó correctamente.
 * \details Crea y configura una ventana para mostrar mensajes, con dimensiones y posición fijas.
 */
int32_t
msg_init(void)
{
    /*
     * Las coordenadas están hardcoded, no se me ocurre una buena forma de
     * hacer automático el cálculo de las dimensiones de las ventanas.
     */
    msg = newwin(10, 110, 0, 0);
    box(msg, 0, 0);
    wrefresh(msg);
    return 0;
}

/**
 * \brief Registra un mensaje en la ventana de logs.
 * \param level Nivel de importancia del mensaje (info, advertencia, error).
 * \param str Cadena de texto que contiene el mensaje a registrar.
 * \return Retorna 0 si el mensaje se registró correctamente.
 * \details Limpia la ventana de logs y muestra el mensaje, con planes futuros para soportar historial y colores.
 */
int32_t
msg_log(enum log_level level, const char *str)
{
    /**
     * Por ahora solo limpiamos la pantalla e imprimimos pero en un futuro
     * deberíamos de ser capaces de mostrar un historial completo de mensajes.
     * TODO: Mostrar un historial de mensajes, no solo el último mensaje
     *       (usar una cola circular).
     */
    clear_window_part(msg, 1, 1, 8, 78);
    /**
     * TODO: Imprimir con colores los mensajes de info, advertencia y error.
     */
    mvwprintw(msg, 1, 2, "%s", str);
    wrefresh(msg);
    return 0;
}