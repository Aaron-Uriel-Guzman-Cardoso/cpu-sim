#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cpu.h>
#include <prog.h>




/**
 * \brief Algoritmo djb2 para la generación de hashes no criptográficos.
 * 
 * Esta función es usada aquí para encontrar programas duplicados
 * en la memoria, evitando así cargar el mismo programa varias veces e
 * identificar procesos hermanos.
 * 
 * Explicación simple: sumas un byte a un hash inicial (5381 es el 
 * específicado por el algoritmo) y lo multiplicas por 33. Una suma de
 * simple de bytes puede dar problemas para programas con líneas intercambiadas,
 * por lo que el 33 le da más peso a las últimas líneas del programa.
 * 
 * \note Este algoritmo no es perfecto puede dar colsiones (muy poco probable)
 *       por lo que hay que hacer unas cuantas verificaciones adicionales.
 * 
 */
unsigned long djb2(const void *mem, size_t len)
{
    /**
     * No se por qué usa primos la verdad xd.
     */
    uint64_t hash = 5381;
    const unsigned char *data = (const unsigned char *)mem;
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + data[i]; // hash * 33 + data[i]
    }
    return hash;
}

/**
 * \brief Crea un programa a partir de un archivo.
 * 
 * Este primer paso es necesario para futuro procesamiento y creación de un
 * proceso de forma adecuada, aquí se realizará el análisis sintáctico del
 * código y su conversión a un formato legible por la CPU.
 * Reglas:
 * - Instrucciones no válidas son tomadas como NOP, para preservar la validez
 *   de los saltos.
 * - Comentarios empiezan con '#' o ';' y son de una sola línea, ignorados.
 * - Las instrucciones JMZ harán un salto a la dirección tomando de referencia
 *   la posición final en memoria, ignorando comentarios pero no instrucciones
 *   inválidas.
 * \param filename Nombre del archivo a cargar.
 * \return Un puntero a la estructura del programa creado, o NULL si hubo un error.
 */
struct prog *
prog_new(const char *filename)
{
    // Abrir el archivo
    FILE *file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    /**
     * Nuestras instrucciones son 4 veces más pequeñas que las que específico 
     * el profesor (8 bytes vs 32 bytes).
     */
    size_t aprox_prog_length = (ftell(file) / (WORD_SIZE * 4)) + 1;
    fseek(file, 0, SEEK_SET);
    struct prog *new_prog = malloc(sizeof(struct prog) + aprox_prog_length * sizeof(struct inst));
    if (!new_prog) {
        fclose(file);
        return NULL;
    }
    new_prog->filename = strdup(filename);
    new_prog->hash = 0;
    new_prog->length = 0;

    char buffer[256]; /*< Nos aseguramos de leer una línea entera */
    while (fgets(buffer, sizeof(buffer), file)) {
        /* Omitimos líneas de comentarios y vacías */
        if (buffer[0] == '#' || buffer[0] == ';' || buffer[0] == '\n' || 
            buffer[0] == '\0')
        {
            continue;
        }
        char *inline_comment = strpbrk(buffer, "#;");
        if (inline_comment) {
            *inline_comment = '\0'; // Terminar la línea en el comentario
        }
        struct inst *loaded_inst = inst_from_str(buffer);
        if (loaded_inst) {
            new_prog->instmem[new_prog->length++] = *loaded_inst;
            free(loaded_inst);
        } else {
            // Instrucción inválida, agregar NOP
            struct inst nop_inst = { .op = OP_NOP };
            new_prog->instmem[new_prog->length++] = nop_inst;
        }
        /**
         * Si por algún motivo la aproximación falló, duplicamos el espacio
         * disponible.
         */
        if (new_prog->length >= aprox_prog_length) {
            aprox_prog_length *= 2;
            new_prog = realloc(new_prog,
                sizeof(struct prog) + aprox_prog_length * sizeof(struct inst));
            if (!new_prog) {
                fclose(file);
                return NULL;
            }
        }
    }
    new_prog->hash = djb2(new_prog->instmem, new_prog->length * sizeof(struct inst));
    fclose(file);
    return new_prog;
}