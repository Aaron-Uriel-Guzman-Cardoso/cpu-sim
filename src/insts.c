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

int32_t op_mov(struct cpu *self, enum reg ra, enum reg rb);
int32_t op_movi(struct cpu *self, enum reg ra, int imm);
int32_t op_add(struct cpu *self, enum reg ra, enum reg rb);
int32_t op_addi(struct cpu *self, enum reg ra, int imm);
int32_t op_sub(struct cpu *self, enum reg ra, enum reg rb);
int32_t op_subi(struct cpu *self, enum reg ra, int imm);
int32_t op_mul(struct cpu *self, enum reg ra, enum reg rb);
int32_t op_muli(struct cpu *self, enum reg ra, int imm);
int32_t op_div(struct cpu *self, enum reg ra, enum reg rb);
int32_t op_divi(struct cpu *self, enum reg ra, int imm);
int32_t op_inc(struct cpu *self, enum reg ra, int32_t imm);
int32_t op_dec(struct cpu *self, enum reg ra,  int32_t imm);
int32_t op_nop(struct cpu *self, enum reg ra,  int32_t imm);
int32_t op_end(struct cpu *self, enum reg ra,  int32_t imm);

/*
 * Unión que representa los dos formatos que pueden tener las operaciones: de
 * registro a registro e inmediatas (el segundo de sus argumentos es un valor
 * entero).
 */
union op_fn {
    int32_t (*reg_to_reg)(struct cpu *, enum reg, enum reg);
    int32_t (*imm)(struct cpu *, enum reg, int32_t);
};

/*
 * Definimos las instrucciones que podrán ser llamadas por la CPU, no todas
 * están definidas pues algunas son únicamente inmediatas.
 */
union op_fn ops[OP_LIMIT] = {
    { .reg_to_reg = op_mov}, { .reg_to_reg = op_add }, { .reg_to_reg = op_sub},
    { .reg_to_reg = op_mul}, { .reg_to_reg = op_div }, { .imm = op_inc },
    { .imm = op_dec }, { .imm = op_nop }, {.imm = op_end }, {.imm = op_movi },
    { .imm = op_addi }, { .imm = op_subi }, { .imm = op_muli },
    { .imm = op_divi }
};

int32_t
op_mov(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    self->regs[ra] = self->regs[rb];
    return 0;
}

int32_t
op_movi(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->regs[ra] = imm;
    return 0;
}

int32_t
op_add(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    self->regs[ra] += self->regs[rb];
    return 0;
}

int32_t
op_addi(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->regs[ra] += imm;
    return 0;
}


int32_t
op_sub(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    self->regs[ra] -= self->regs[rb];
    return 0;
}

int32_t
op_subi(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->regs[ra] -= imm;
    return 0;
}

int32_t
op_mul(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    self->regs[ra] *= self->regs[rb];
    return 0;
}

int32_t
op_muli(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->regs[ra] *= imm;
    return 0;
}

int32_t
op_div(struct cpu *self, enum reg ra, enum reg rb)
{
    if (ra >= REG_LIMIT || rb >= REG_LIMIT) {
        return 1;
    }
    if (self->regs[rb] == 0) {
        self->regs[ra] = 0;
        return 0;
    }
    self->regs[ra] /= self->regs[rb];
    return 0;
}

int32_t
op_divi(struct cpu *self, enum reg ra, int imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    if (imm == 0) {
        self->regs[ra] = 0;
        return 0;
    }
    self->regs[ra] /= imm;
    return 0;
}

int32_t
op_inc(struct cpu *self, enum reg ra, int32_t imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->regs[ra] += 1;
    return 0;
}

int32_t
op_dec(struct cpu *self, enum reg ra, int32_t imm)
{
    if (ra >= REG_LIMIT) {
        return 1;
    }
    self->regs[ra] -= 1;
    return 0;
}

int32_t
op_nop(struct cpu *self, enum reg ra, int32_t imm)
{
    if (ra >= REG_LIMIT) {
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

/*
 * Función para procesar la instrucción END, por algún motivo si esta se
 * llamaba end se le asignaba un valor erroneo.
 */
int32_t 
op_end(struct cpu *self, enum reg ra, int32_t imm)
{
    self->state = CPU_HALT;
    return 1;
}

/*
 * Convierte str a un formato de instrucción procesable por la CPU.
 * En caso de no haber una conversión válida se regresará NULL.
 */
struct inst *
inst_from_str(const char *buf)
{
    if (!buf || buf[0] == '\0') {
        return NULL;
    }
    char name[5], arg1[5], arg2[5];
    int32_t args = sscanf(buf, "%4s %4s %4s", name, arg1, arg2);
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
            /*
             * Verificamos que arg2 sea o no un número, en caso de no serlo
             * strtol debería de no mover el apuntador end y en caso de ser un
             * número el mismo strtol debería de haberlo movido al final de la
             * cadena (arg2 no tendrá nada después del número).
             */
            char *end;
            new_inst->imm = strtol(arg2, &end, 10);
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
 * Ejecuta las instrucciones, regresará 1 cuando la instrucción a ejecutar sea
 * simplemente END.
 */
int32_t
inst_execute(struct inst *self, struct cpu *cpu)
{
    if (!self || !cpu || self->op >= OP_LIMIT) { return 2; }
    if (self->op >= OP_INC && self->op <= OP_LIMIT) {
        return ops[self->op].imm(cpu, self->ra, self->imm);
    } else {
        return ops[self->op].reg_to_reg(cpu, self->ra, self->rb);
    }
    /*
     * Esto nunca debería de pasar
     */
    return 2;
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
