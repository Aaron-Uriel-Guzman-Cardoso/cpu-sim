#include <ncurses_utilities.h>

/*
 * Prototipo: void clear_window_part(WINDOW *win, int start_y, int start_x, int height, int width);
 * Propósito: Limpiar una parte específica de una ventana sin borrar el marco
 * Entradas: Puntero de tipo ventana de ncurses (WINDOW *win), variables de tipo entero (start_y, 
 * start_x, height, width)
 * Salidas: Ninguna
 * Descripción: Mediante bucles anidados va recorriendo la venta a lo ancho y alto (con límites especi-
 * ficados por height y width) y por cada iteración mvwaddch coloca un caracter de espacio vacío ' ', 
 * reemplazando los caracteres que estan en esas coordenadas. Finalmente wrefresh refresca la ventana 
 * reflejando los cambios.
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
