#include <assert.h>
#include <cpu.h>
#include <stdint.h>

/*
 * Definimos los programas como arreglos y no como strings literales porque
 * estos serán manipulados (ver uso de strtok_r en el código de cpu).
 */
char prog_one[] = "MoV Ax -13\n"
                  "add bx 31\n"
                  "inc ax\n"
                  "inc ax\n"
                  "dec bx\n"
                  "Mul cX 3140\n"
                  "aDD ax 10\n"
                  "div bx ax\n"
                  "end\n"
                  "MOV RAX 3\n";

char prog_two[] =
    "MOV AX 0000\n"
    "MOV BX 0000\n"
    "MOV CX 0000\n"
    "MOV DX 0000\n"
    "add AX 5\n"
    "add AX 5\n"
    "add AX 5\n"
    "add AX 5\n"
    "add AX 5\n"
    "add AX 5\n"
    "add AX 5\n"
    "add AX 5\n"
    "add AX 5\n"
    "add AX 5\n"
    "mov BX AX\n"
    "mov cX AX\n"
    "mov dX AX\n"
    "mul AX 4\n"
    "mul BX 4\n"
    "mul CX 4\n"
    "mul DX 4\n"
    "DIV ax 2\n"
    "DIV Bx 2\n"
    "DIV cx 2\n"
    "DIV dx 2\n"
    "inc ax\n"
    "add bx 2\n"
    "DIV cx 2\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "dec Cx\n"
    "Mul cx -3\n"
    "ADD cx 100\n"
    "SuB dx 4\n"
    "inc dx\n"
    "inc dx\n"
    "inc dx\n"
    "inc dx\n"
    "inc dx\n"
    "inc dx\n"
    "inc dx\n"
    "inc dx\n"
    "end\n"
    "MOV AX 0000\n"
    "MOV BX 0000\n"
    "MOV CX 0000\n"
    "MOV DX 0000\n";
int
main(void)
{
    struct cpu *cpu = cpu_new();
    cpu_load_insts_from_str(cpu, prog_one);
    for (int32_t i = 0; i < 8; i += 1) {
        assert(cpu_next_cycle(cpu) == 0);
        assert(cpu->state == CPU_READY);
    }
    assert(cpu_next_cycle(cpu) == 3); /* La instrucción ejecutada es END*/
    assert(cpu->state == CPU_HALT);
    assert(cpu_next_cycle(cpu) == 0);
    assert(cpu->state == CPU_HALT);

    cpu_reset(cpu);
    cpu_load_insts_from_str(cpu, prog_two);
    for (int32_t i = 0; i < 90; i += 1) {
        assert(cpu_next_cycle(cpu) == 0);
        assert(cpu->state == CPU_READY);
    }
    assert(cpu_next_cycle(cpu) == 3);
    assert(cpu->state == CPU_HALT);
    return 0;
}
