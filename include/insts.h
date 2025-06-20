#ifndef INSTS_H
#define INSTS_H

#include <stddef.h>
#include <stdint.h>

/*
 * Hay que buscarles mejor hogar a estas enumeraciones xd, me da problema si
 * las pongo en cpu.h
 */
enum op {
    OP_MOV, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_INC, OP_DEC, OP_NOP, OP_END,
    OP_MOVI, OP_ADDI, OP_SUBI, OP_MULI, OP_DIVI, OP_LIMIT
};
enum reg { REG_AX, REG_BX, REG_CX, REG_DX, REG_PC, REG_IR, REG_LIMIT};

/*
 * Estructura que representa a una instrucción ejecutable por la CPU.
 * Se trata también de una unión etiquetada, donde la etiqueta es si una
 * instrucción es de registro a registro o inmediata.
 */
struct inst {
    enum op op: 6;
    enum reg ra: 6;
    union {
        enum reg rb: 6;
        int32_t imm; 
    };
};
struct inst *inst_from_str(char *buf);
void inst_to_str(const struct inst *self, char *str, size_t size);

#endif
