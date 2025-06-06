#include <ncurses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses_utilities.h>
#include <cmd.h>

#include <prompt.h>

#define PROMPT_HISTORY_SIZE 4
#define MAX_BUFLEN 76 /* Esto es lo que cabe en pantalla */

struct history {
    char raw_history[PROMPT_HISTORY_SIZE][MAX_BUFLEN];
    int current;
    int size;
};

// Estructura que representa el estado del prompt de comandos en la TUI
struct prompt {
    char buf[MAX_BUFLEN];
    size_t buflen;
    WINDOW *win;
    struct history hist;
    int hist_index;
    struct cmd *cmd;
} *prompt;

/**
 * \brief Inicializa la estructura y ventana del prompt.
 *
 * Reserva memoria para el prompt, inicializa sus campos y crea la ventana ncurses.
 * Dibuja el borde, el título y actualiza la visualización inicial.
 */
void
prompt_init(void)
{
    prompt = malloc(sizeof(*prompt));
    if (!prompt) {
        perror("Error al inicializar el prompt");
        exit(EXIT_FAILURE);
    }
    prompt->buf[0] = '\0';
    prompt->buflen = 0;
    prompt->win = newwin(7, 110, 17, 0);
    box(prompt->win, 0, 0);
    prompt->hist.current = 0;
    prompt->hist.size = 0;
    prompt->hist_index = 0;
    prompt->cmd = NULL;

    mvwprintw(prompt->win, 0, 35, "|Prompt|");
    wrefresh(prompt->win);

    prompt_update();
}

/**
 * \brief Limpia la pantalla de línea de comandos, eliminando también el
 *        historial
 */
void
prompt_clear()
{
    clear_window_part(prompt->win, 1, 1, 5, 78);
    prompt->buf[0] = '\0';
    prompt->buflen = 0;
    prompt->hist_index = 0;
    /**
     * TODO: borrar el historial al reiniciar la línea de comandos
     */
    wrefresh(prompt->win);
}

/**
 * \brief Inserta un carácter en el buffer del prompt.
 *
 * Si no se ha alcanzado el límite, agrega el carácter al buffer y actualiza la pantalla.
 */
void
prompt_insert_char(char ch)
{
    if (prompt->buflen >= MAX_BUFLEN - 1) {
        return;
    }
    prompt->buf[prompt->buflen] = ch;
    prompt->buf[prompt->buflen + 1] = 0;
    prompt->buflen += 1;
    prompt_update();
}

/**
 * \brief Elimina el último carácter del buffer del prompt (backspace).
 *
 * Si el buffer no está vacío, elimina el último carácter y actualiza la pantalla.
 */
void
prompt_backspace(void)
{
    if (prompt->buflen > 0) {
        prompt->buflen -= 1;
        prompt->buf[prompt->buflen] = '\0';
    }
    prompt_update();
}

/**
 * \brief Procesa la entrada cuando el usuario presiona Enter.
 *
 * Limpia la zona de comandos, guarda el comando en el historial,
 * decodifica el comando, actualiza los índices y limpia el buffer.
 */
void
prompt_enter(void)
{
    clear_window_part(prompt->win, 1, 1, 5, 78);
    strncpy(prompt->hist.raw_history[prompt->hist.current], prompt->buf, prompt->buflen);
    prompt->cmd = cmd_decode(prompt->buf);

    prompt->hist.current = (prompt->hist.current + 1) % PROMPT_HISTORY_SIZE;
    if (prompt->hist.size < PROMPT_HISTORY_SIZE) {
        prompt->hist.size++;
    }
    prompt->hist_index = prompt->hist.current;

    prompt->buf[0] = prompt->buflen = 0;
    prompt_update();
}

bool
prompt_valid_cmd_is_entered(void)
{
    return (prompt->cmd);
}

/**
 * \brief Obtiene el comando ingresado por el usuario
 * 
 * Esta función regresa el comando que fue ingresado por el usuario, si no hay
 * un comando válido se regresa NULL.
 * 
 * Nota: esta función limpia el comando de la memoria del prompt, por lo que
 *       el comando que se ingresó solo se puede obtener de prompt una vez,
 *       el usuario tiene que repetirlo.
 * 
 * \return Puntero al comando ingresado o NULL si no hay un comando válido.
 */
struct cmd *
prompt_get_cmd(void)
{
    struct cmd *cmd = prompt->cmd;
    prompt->cmd = NULL;
    return cmd;
}

/**
 * \brief Realiza una actualización del estado del prompt de forma que se
 *        refleja en la TUI
 * 
 * Esta función es requerida cuando algo en el estado interno del prompt
 * fue cambiado y se requiere de una impresión actualizada del prompt.
 * Usualmente esto sucede cuando te mueves por el historial o ingresaste
 * un nuevo comando.
 */
void
prompt_update(void)
{
    clear_window_part(prompt->win, 5, 1, 1, 78);
    for (int i = 0; i < prompt->hist.size; i++) {
        size_t index = 
            (prompt->hist.current - 1 - i + PROMPT_HISTORY_SIZE) % PROMPT_HISTORY_SIZE;
        mvwprintw(prompt->win, 4 - i, 1, "$ %s", prompt->hist.raw_history[index]);
    }
    mvwprintw(prompt->win, 5, 1, "$ %s", prompt->buf);
    wrefresh(prompt->win);
}

/**
 * \brief Navega hacia arriba en el historial de comandos del prompt.
 *
 * Si hay comandos en el historial, decrementa el índice del historial (de forma circular),
 * copia el comando correspondiente al buffer del prompt y actualiza la pantalla.
 */
void
prompt_up(void)
{
    if (prompt->hist.size > 0) {
        prompt->hist_index = 
            (prompt->hist_index - 1 + PROMPT_HISTORY_SIZE) % PROMPT_HISTORY_SIZE;
        prompt->buflen = strnlen(prompt->hist.raw_history[prompt->hist_index], MAX_BUFLEN) + 1;
        snprintf(prompt->buf, prompt->buflen, "%s",
                 prompt->hist.raw_history[prompt->hist_index]);
        prompt_update();
    }
}

/**
 * \brief Navega hacia abajo en el historial de comandos del prompt.
 *
 * Si hay comandos en el historial, incrementa el índice del historial (de forma circular),
 * copia el comando correspondiente al buffer del prompt y actualiza la pantalla.
 */
void
prompt_dn(void)
{
    if (prompt->hist.size > 0) {
        prompt->hist_index = (prompt->hist_index + 1) % PROMPT_HISTORY_SIZE;
        prompt->buflen = strnlen(prompt->hist.raw_history[prompt->hist_index], MAX_BUFLEN) + 1;
        snprintf(prompt->buf, prompt->buflen, "%s",
                 prompt->hist.raw_history[prompt->hist_index]);
        prompt_update();
    }
}