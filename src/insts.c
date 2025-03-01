//#include <insts.h>
#include "../include/insts.h"
#include <stdint.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>

/*
 * Estructura que representa a una instrucción ejecutable por la CPU.
 * Se trata también de una unión etiquetada, donde la etiqueta es si una
 * instrucción es de registro a registro o inmediata.
 */
struct inst {
    enum op op;
    enum reg ra;
    enum { INST_IMMEDIATE, INST_REGISTER } type;
    union {
        enum reg rb;
        int32_t imm;
    };
};

int32_t is_numeric(const char *str);

enum reg reg_from_str(const char *str);
enum op op_from_str(const char *str);

int32_t mov(struct cpu *self, enum reg ra, enum reg rb);
int32_t movi(struct cpu *self, enum reg ra, int imm);
int32_t add(struct cpu *self, enum reg ra, enum reg rb);
int32_t addi(struct cpu *self, enum reg ra, int imm);
int32_t sub(struct cpu *self, enum reg ra, enum reg rb);
int32_t subi(struct cpu *self, enum reg ra, int imm);
int32_t mul(struct cpu *self, enum reg ra, enum reg rb);
int32_t muli(struct cpu *self, enum reg ra, int imm);
int32_t cdiv(struct cpu *self, enum reg ra, enum reg rb);
int32_t divi(struct cpu *self, enum reg ra, int imm);
int32_t inc(struct cpu *self, enum reg ra, int32_t imm);
int32_t dec(struct cpu *self, enum reg ra, int32_t imm);
int32_t nop(struct cpu *self, enum reg ra, enum reg rb);
int32_t nopi(struct cpu *self, enum reg ra, int32_t imm);
int32_t end(struct cpu *self, enum reg ra, int32_t imm);

/*
 * Definimos las instrucciones que podrán ser llamadas por la CPU, no todas
 * están definidas pues algunas son únicamente inmediatas.
 */
int32_t (*insts[OP_LIMIT])(struct cpu *, enum reg, enum reg) = {
    mov, add, sub, mul, cdiv, nop, nop, nop, nop
};

int32_t (*imm_insts[OP_LIMIT])(struct cpu *, enum reg, int) = {
    movi, addi, subi, muli, divi, inc, dec, nopi, end
};

int32_t
mov(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    self->int_regs[ra] = self->int_regs[rb];
    return 0;
}

int32_t
movi(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->int_regs[ra] = imm;
    return 0;
}

int32_t
add(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    self->int_regs[ra] += self->int_regs[rb];
    return 0;
}

int32_t
addi(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->int_regs[ra] += imm;
    return 0;
}


int32_t
sub(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    self->int_regs[ra] -= self->int_regs[rb];
    return 0;
}

int32_t
subi(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->int_regs[ra] -= imm;
    return 0;
}

int32_t
mul(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    self->int_regs[ra] *= self->int_regs[rb];
    return 0;
}

int32_t
muli(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->int_regs[ra] *= imm;
    return 0;
}

int32_t
cdiv(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    self->int_regs[ra] /= self->int_regs[rb];
    return 0;
}

int32_t
divi(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->int_regs[ra] /= imm;
    return 0;
}

int32_t
inc(struct cpu *self, enum reg ra, int32_t imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->int_regs[ra] += 1;
    return 0;
}

int32_t
dec(struct cpu *self, enum reg ra, int32_t imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->int_regs[ra] -= 1;
    return 0;
}

int32_t
nop(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    return 0;
}

int32_t
nopi(struct cpu *self, enum reg ra, int32_t imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    return 0;
}

int32_t 
end(struct cpu *self, enum reg ra, int32_t imm)
{
    return 1;
}

/*
 * Convierte str en una instrucción procesable por la CPU.
 * En caso de no haber una conversión válida se regresará NULL.
 * Esta es la que más trabajo va a ocupar de desarrollo
 */
struct inst *
inst_decode(const char *buf)
{
    char name[5], arg1[5], arg2[5];
    int32_t args = sscanf(buf, "%4s %4s %4s", name, arg1, arg2);
    struct inst *new_inst = malloc(sizeof(*new_inst));
    if (new_inst && args >= 1) {
        new_inst->op = op_from_str(name);
        if (new_inst->op == OP_LIMIT) {
            return NULL;
        }
        if (args >= 2) {
            if(is_numeric(arg1)) {
                return NULL;
            }
            new_inst->ra = reg_from_str(arg1);
            if (args == 3) {
                if (is_numeric(arg2)) {
                    new_inst->type = INST_IMMEDIATE;
                    new_inst->imm = atoi(arg2);
                } else {
                    new_inst->type = INST_REGISTER;
                    new_inst->rb = reg_from_str(arg2);
                    if (new_inst->rb == REG_LIMIT) {
                        return NULL;
                    }
                }
            } else if (args == 2 && (new_inst->op == OP_INC || new_inst->op == OP_DEC)) {
                new_inst->type = INST_IMMEDIATE;
                new_inst->imm = 0;
            } else {
                return NULL;
            }
        } else if (args == 1 && new_inst->op == OP_END) {
            new_inst->type = INST_IMMEDIATE;
            new_inst->ra = REG_AX;
            new_inst->imm = 0;
        }
    }
    return new_inst;
}

int32_t
inst_execute(struct inst *self, struct cpu *cpu)
{
    if (self->type == INST_IMMEDIATE) {
        return imm_insts[self->op](cpu, self->ra, self->imm); 
    } else 
    if (self->type == INST_REGISTER) {
        return insts[self->op](cpu, self->ra, self->rb);
    }
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

/*
 * Obtiene la correspondencia de uso interno entre str y una operación
 * realizada por la CPU.
 */
enum op
op_from_str(const char *str)
{
    if (strncmp(str, "MOV", 3) == 0) {
        return OP_MOV;
    }
    else if (strncmp(str, "ADD", 3) == 0) {
        return OP_ADD;
    }
    else if (strncmp(str, "SUB", 3) == 0) {
        return OP_SUB;
    }
    else if (strncmp(str, "MUL", 3) == 0) {
        return OP_MUL;
    }
    else if (strncmp(str, "DIV", 3) == 0) {
        return OP_DIV;
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

/*
 * Verificación de si una cadena es un número o no a través de expresiones
 * regulares.
 */
int32_t is_numeric(const char *str)
{
    regex_t regex;
    int reti;

    // Compilar la expresión regular para números
    reti = regcomp(&regex, "^[+-]?[0-9]+$", REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "No se pudo compilar la expresión regular\n");
        return -1;
    }

    // Ejecutar la expresión regular
    reti = regexec(&regex, str, 0, NULL, 0);
    regfree(&regex);

    if (!reti) {
        return 1; // La cadena es numérica
    } else if (reti == REG_NOMATCH) {
        return 0; // La cadena no es numérica
    } else {
        fprintf(stderr, "Error al ejecutar la expresión regular\n");
        return -1;
    }
}

