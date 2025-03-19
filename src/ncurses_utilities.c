#include <ncurses_utilities.h>

/**
 * \brief Limpia una parte específica de una ventana sin borrar el marco.
 *
 * Esta función recorre una sección rectangular de una ventana de ncurses y reemplaza
 * todos los caracteres en esa área con espacios en blanco (' '). Esto permite limpiar
 * una parte específica de la ventana sin afectar el resto de su contenido o su marco.
 *
 * \param win Puntero a la ventana de ncurses que se desea modificar.
 * \param start_y Coordenada Y inicial desde donde comenzar a limpiar.
 * \param start_x Coordenada X inicial desde donde comenzar a limpiar.
 * \param height Altura del área rectangular que se desea limpiar.
 * \param width Ancho del área rectangular que se desea limpiar.
 *
 * \return No devuelve ningún valor (void).
 */

void
clear_window_part(WINDOW *win, int start_y, int start_x, int height, int width)
{
    for (int y = start_y; y < start_y + height; y++) {
        for (int x = start_x; x < start_x + width; x++) {
            mvwaddch(win, y, x, ' ');
        }
    }
    wrefresh(win);
}
