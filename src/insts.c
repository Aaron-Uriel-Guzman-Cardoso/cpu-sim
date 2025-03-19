#include <insts.h>
#include <cpu.h>
#include <stdint.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdbool.h>


enum reg reg_from_str(const char *str);
enum op op_from_str(const char *str, bool is_immediate);

/**
 * \brief Convierte una cadena de texto en una estructura de instrucción.
 * \param buf Cadena de texto que contiene la instrucción a convertir.
 * \return Retorna un puntero a la estructura `inst` creada o NULL si la conversión falla.
 * \details Analiza la cadena de entrada, extrae el nombre de la operación y los argumentos,
 *         y los convierte en una estructura de instrucción válida.
 */

struct inst *
inst_from_str(const char *buf)
{
    if (!buf || buf[0] == '\0') {
        return NULL;
    }
    char name[5], arg1[5];
    char arg2[11]; /* arg2 puede ser un número de hasta 10 dígitos*/
    int32_t args = sscanf(buf, "%4s %4s %10s", name, arg1, arg2);
    struct inst *new_inst = malloc(sizeof(*new_inst));
    if (new_inst) {
        if (args == 1 || args == 2) {
            new_inst->op = op_from_str(name, true);
            if (new_inst->op == OP_LIMIT) {
                goto err;
            }
            new_inst->ra = REG_AX;
        }
        if (args >= 2) {
            new_inst->ra = reg_from_str(arg1);
            if (new_inst->ra == REG_LIMIT) {
                goto err;
            }
            /*
             * Inicializamos con cero en caso de que la instrucción sea de
             * solamente dos argumentos o inmediata.
             */
            new_inst->imm = 0;
        }
        if (args == 3) {
            /**
             * Verificamos que arg2 sea o no un número:
             * - En caso de no serlo strtol debería de no mover el apuntador
             *   end.
             * - En caso de ser un número el mismo strtol debería de haberlo
             *   movido al final de la cadena (arg2 no tendrá nada después del
             *   número).
             * 
             * Dado que strol es para longs, nos aseguramos de que no sea
             * introducido un valor más grande que los que soporte la CPU, que
             * son ints de 32 bits.
             */
            char *end;
            long tmp = strtol(arg2, &end, 10);
            if (tmp > INT32_MAX || tmp < INT32_MIN) {
                goto err;
            }
            new_inst->imm = tmp;
            if (end == arg2 || *end != '\0') {
                new_inst->op = op_from_str(name, false);
                new_inst->rb = reg_from_str(arg2);
            } else {
                new_inst->op = op_from_str(name, true);
            }
        }
    }
    return new_inst;
    /*
     * Aquí está lo que manejamos cuando la instrucción no pudo ser
     * decodificada.
     */
err:
    free(new_inst);
    return NULL;
}


/**
 * \brief Convierte una cadena de texto en un identificador de registro.
 * \param str Cadena de texto que representa el nombre del registro.
 * \return Retorna el identificador del registro correspondiente o REG_LIMIT si no hay coincidencia.
 * \details Compara la cadena de entrada con los nombres de los registros conocidos (AX, BX, CX, DX, PC, IR)
 *         y devuelve el identificador correspondiente.
 */
enum reg
reg_from_str(const char *str)
{
    if (strncmp(str, "AX", 2) == 0) {
        return REG_AX;
    }
    else if (strncmp(str, "BX", 2) == 0) {
        return REG_BX;
    }
    else if (strncmp(str, "CX", 2) == 0) {
        return REG_CX;
    }
    else if (strncmp(str, "DX", 2) == 0) {
        return REG_DX;
    }
    else if (strncmp(str, "PC", 2) == 0) {
        return REG_PC;
    }
    else if (strncmp(str, "IR", 2) == 0) {
        return REG_IR;
    }
    else {
        /*
         * Regresamos un registro que no existe cuando no hay conversión
         * válida. 
         */
        return REG_LIMIT;
    }
}

/**
 * \brief Convierte un identificador de registro en una cadena de texto.
 * \param reg Identificador del registro que se desea convertir.
 * \param str Puntero al buffer donde se almacenará la cadena resultante.
 * \param size Tamaño del buffer.
 * \return No devuelve ningún valor (void).
 * \details Convierte el identificador de registro en su representación en cadena de texto (AX, BX, CX, etc.).
 */
void
reg_to_str(enum reg reg, char *str, size_t size)
{
    if (!str || size == 0) {
        return;
    }
    switch (reg) {
    case REG_AX:
        snprintf(str, size, "AX");
        break;
    case REG_BX:
        snprintf(str, size, "BX");
        break;
    case REG_CX:
        snprintf(str, size, "CX");
        break;
    case REG_DX:
        snprintf(str, size, "DX");
        break;
    case REG_PC:
        snprintf(str, size, "PC");
        break;
    case REG_IR:
        snprintf(str, size, "IR");
        break;
    default:
        snprintf(str, size, "LIMIT");
        break;
    }
}

/**
 * \brief Convierte una cadena de texto en un identificador de operación.
 * \param str Cadena de texto que representa el nombre de la operación.
 * \param is_immediate Indica si la operación es inmediata (usa un valor directo).
 * \return Retorna el identificador de la operación correspondiente o OP_LIMIT si no hay coincidencia.
 * \details Compara la cadena de entrada con los nombres de las operaciones conocidas (MOV, ADD, SUB, etc.)
 *         y devuelve el identificador correspondiente, considerando si es una operación inmediata o no.
 */
enum op
op_from_str(const char *str, bool is_immediate)
{
    if (strncmp(str, "MOV", 3) == 0) {
        return (is_immediate)? OP_MOVI: OP_MOV;
    }
    else if (strncmp(str, "ADD", 3) == 0) {
        return (is_immediate)? OP_ADDI: OP_ADD;
    }
    else if (strncmp(str, "SUB", 3) == 0) {
        return (is_immediate)? OP_SUBI: OP_SUB;
    }
    else if (strncmp(str, "MUL", 3) == 0) {
        return (is_immediate)? OP_MULI: OP_MUL;
    }
    else if (strncmp(str, "DIV", 3) == 0) {
        return (is_immediate)? OP_DIVI: OP_DIV;
    }
    else if (strncmp(str, "INC", 3) == 0) {
        return OP_INC;
    }
    else if (strncmp(str, "DEC", 3) == 0) {
        return OP_DEC;
    }
    else if (strncmp(str, "END", 3) == 0) {
        return OP_END;
    }
    else {
        /*
         * Operación que no existe en caso de que no se haya hecho match con
         * ninguna operación.
         */
        return OP_LIMIT;
    }
}

/**
 * \brief Convierte un identificador de operación en una cadena de texto.
 * \param op Identificador de la operación que se desea convertir.
 * \param str Puntero al buffer donde se almacenará la cadena resultante.
 * \param size_t Tamaño del buffer.
 * \return No devuelve ningún valor (void).
 * \details Convierte el identificador de operación en su representación en cadena de texto (MOV, ADD, SUB, etc.).
 */
void
op_to_str(enum op op, char *str, size_t size_t)
{
    if (!str || size_t == 0) {
        return;
    }
    switch (op) {
    case OP_MOV: case OP_MOVI:
        snprintf(str, size_t, "MOV");
        break;
    case OP_ADD: case OP_ADDI:
        snprintf(str, size_t, "ADD");
        break;
    case OP_SUB: case OP_SUBI:
        snprintf(str, size_t, "SUB");
        break;
    case OP_MUL: case OP_MULI:
        snprintf(str, size_t, "MUL");
        break;
    case OP_DIV: case OP_DIVI:
        snprintf(str, size_t, "DIV");
        break;
    case OP_INC:
        snprintf(str, size_t, "INC");
        break;
    case OP_DEC:
        snprintf(str, size_t, "DEC");
        break;
    case OP_END:
        snprintf(str, size_t, "END");
        break;
    default:
        snprintf(str, size_t, "LIMIT");
        break;
    }
}

/**
 * \brief Determina si una operación es inmediata.
 * \param op Identificador de la operación que se desea verificar.
 * \return Retorna `true` si la operación es inmediata, `false` en caso contrario.
 * \details Verifica si la operación dada es de tipo inmediato (usa un valor directo).
 */
bool
is_immediate(enum op op)
{
    return (op >= OP_INC && op < OP_LIMIT);
}

/**
 * \brief Convierte una estructura de instrucción en una cadena de texto.
 * \param self Puntero a la estructura de instrucción que se desea convertir.
 * \param str Puntero al buffer donde se almacenará la cadena resultante.
 * \param size Tamaño del buffer.
 * \return No devuelve ningún valor (void).
 * \details Convierte la instrucción en una cadena de texto legible, incluyendo la operación y sus argumentos.
 */
void
inst_to_str(const struct inst *self, char *str, size_t size) {
    char op[6], arg1[6], arg2[6];
    op_to_str(self->op, op, 6);
    reg_to_str(self->ra, arg1, 6);
    if (is_immediate(self->op)) {
        snprintf(arg2, 6, "%d", self->imm);
    } else {
        reg_to_str(self->rb, arg2, 6);
    }
    /*
     * Hacemos cadena vacía a los argumentos para las operaciones no toman a
     * dichos argumentos.
     */
    if (self->op == OP_END) {
        arg1[0] = '\0';
        arg2[0] = '\0';
    }
    else if (self->op == OP_INC || self->op == OP_DEC) {
        arg2[0] = '\0';
    } 
    snprintf(str, size, "%s %s %s", op, arg1, arg2);
}
