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
 * \brief Convierte str a un formato de instrucción procesable por la CPU.
 * 
 * En caso de no haber una conversión válida se regresará NULL.
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


/*
 * Analiza str y devuelve el valor de uso interno del registro al que se
 * refiere.
 * Por ejemplo, la cadena "AX" corresponderá con REG_AX.
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

/*
 * Identifica la operación que efectuará la instrucción leída, esta función
 * regresará entonces la operación en un formato que se puede manipular más
 * fácilmente para el procesado de la CPU.
 * El booleano `is_immediate` determinará si se usará la variante inmediata de
 * la operación, en caso de no haber variante inmediata se regresará la misma
 * función.
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

/*
 * Determina si la operación recibida es de tipo inmediato
 */
bool
is_immediate(enum op op)
{
    return (op >= OP_INC && op < OP_LIMIT);
}

/*
 * Convierte una instrucción en un formato legible para el ser humano.
 * En este caso LIMIT significa que un valor no pudo ser traducido.
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
