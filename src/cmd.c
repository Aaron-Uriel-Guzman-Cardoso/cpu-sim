#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <cmd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>


/**
 * \brief Decodifica una cadena de texto en una estructura de comando.
 *
 * Esta función toma una cadena de texto (`buf`) que representa una instrucción,
 * la divide en componentes (nombre del comando y argumentos), y la almacena en
 * una estructura.
 * 
 * TODO: Asegurarse desde esta etapa la validez del comando ingresado consultando la
 *       lista de comandos válidos.
 * TODO: Aceptar nombres de archivos con mayúsculas y minúsculas.
 *
 * \param buf Cadena de texto que contiene la instrucción a decodificar.
 *
 * \return Retorna un puntero a la estructura `cmd` que contiene la instrucción decodificada.
 *         Retorna `NULL` si no se pudo asignar memoria para la estructura.
 */
struct cmd *
cmd_decode(const char *buf)
{
    struct cmd *inst = malloc(sizeof(*inst));
    if (inst) {
        memset(inst->name, 0, sizeof(inst->name));
        memset(inst->arg1, 0, sizeof(inst->arg1));
        memset(inst->arg2, 0, sizeof(inst->arg2));

        sscanf(buf, "%s %s %s", inst->name, inst->arg1, inst->arg2);
        for(size_t i = 0; inst->name[i] != '\0'; i += 1) {
            inst->name[i] = toupper(inst->name[i]);
        }
        for(size_t i = 0; inst->arg1[i] != '\0'; i += 1) {
            inst->arg1[i] = toupper(inst->arg1[i]);
        }
        for(size_t i = 0; inst->arg2[i] != '\0'; i += 1) {
            inst->arg2[i] = toupper(inst->arg2[i]);
        }
    }
    return inst;
}

