#ifndef INSTS_H
#define INSTS_H

#include <stdint.h>
//#include <cpu.h>
#include "../include/cpu.h"

enum op { OP_MOV, OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_INC, OP_DEC, OP_NOP, OP_END, OP_LIMIT};

struct inst;
struct inst * inst_decode(const char *buf);
int32_t inst_execute(struct inst *self, struct cpu *cpu);

#endif
