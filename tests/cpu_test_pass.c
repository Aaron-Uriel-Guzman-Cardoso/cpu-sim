#include "insts.h"
#include <assert.h>
#include <cpu.h>
#include <stdint.h>
#include <string.h>

/*
 * Definimos los programas como una sola cadena y no como strings literales 
 * pues uso strtok_r y este manipula memoria para tokenizar cadenas.
 */
char test_prog[] = "mov ax -12\n"
                  "add bx +31\n"
                  "inc ax\n"
                  "inc ax\n"
                  "dec bx\n"
                  "sub bx ax\n"
                  "mov cx bx\n"
                  "mul cx 50\n"
                  "mov dx cx\n"
                  "div cx 0\n"
                  "div dx 7\n"
                  "add dx 2147483638\n"
                  "end\n";
int
main(void)
{
    struct cpu *cpu = cpu_new();
    cpu_load_insts_from_str(cpu, test_prog);
    /*
     * Verificamos que la CPU esté correctamente inicializada haciendo un
     * volcado de su contexto inicial y comparándolo con lo que esperamos..
     */
    {
        const struct inst start_inst = {
            .op = OP_MOVI,
            .ra = REG_AX,
            .imm = -12
        };
        struct cpu_context init_context = cpu_dump_context(cpu);
        assert(init_context.regs[REG_AX] == 0);
        assert(init_context.regs[REG_BX] == 0);
        assert(init_context.regs[REG_CX] == 0);
        assert(init_context.regs[REG_DX] == 0);
        assert(init_context.regs[REG_PC] == 1);

        /**
         * Comparamos miembro a miembro de la estructura porque parece que
         * memcmp no lo hace bien :(.
         * TODO: Hacer comparación usando memcmp en lugar de miembro a miebro
         *       para poder ser más genéricos en cuanto al contenido de \struct inst
         */
        struct inst *ir_inst = (struct inst *)&init_context.regs[REG_IR];
        assert(ir_inst->op == start_inst.op);
        assert(ir_inst->ra == start_inst.ra);
        assert(ir_inst->imm == start_inst.imm);
    }
    /**
     * TODO: implementar un mecanismo para que la CPU ejecute todas las
     *       instrucciones de una, para aumentar el determinismo de las pruebas
     *       (en algunas computadoras rápidas no podría alcanzar a ejecutar todo).
     */
    cpu_set_freq(cpu, 1E9); /* CPU a 1 GHz para que acabe rápido. */
    assert(cpu_sync(cpu) == 1);
    /*
     * Consultamos los eventos que sucedieron en cada uno de los ciclos de la
     * CPU.
     */
    assert(cpu_poll_event(cpu) == CPU_INSTRUCTION_EXECUTED);
    assert(cpu_poll_event(cpu) == CPU_INSTRUCTION_EXECUTED);
    assert(cpu_poll_event(cpu) == CPU_INSTRUCTION_EXECUTED);
    assert(cpu_poll_event(cpu) == CPU_INSTRUCTION_EXECUTED);
    assert(cpu_poll_event(cpu) == CPU_INSTRUCTION_EXECUTED);
    assert(cpu_poll_event(cpu) == CPU_INSTRUCTION_EXECUTED);
    assert(cpu_poll_event(cpu) == CPU_INSTRUCTION_EXECUTED);
    assert(cpu_poll_event(cpu) == CPU_INSTRUCTION_EXECUTED);
    assert(cpu_poll_event(cpu) == CPU_INSTRUCTION_EXECUTED);
    assert(cpu_poll_event(cpu) == CPU_DIVISION_BY_ZERO);
    assert(cpu_poll_event(cpu) == CPU_INSTRUCTION_EXECUTED);
    assert(cpu_poll_event(cpu) == CPU_REGISTER_OVERFLOW);
    assert(cpu_poll_event(cpu) == CPU_HALT);
    assert(cpu_poll_event(cpu) == CPU_NONE);

    /*
     * Consultamos el estado al finalizar la ejecución de la CPU, conocemos
     * los valores así que probaremos que sean los esperados.
     */
    {
        const struct inst end_inst = {
            .op = OP_END,
            .ra = REG_AX,
            .imm = 0
        };
        struct cpu_context end_context = cpu_dump_context(cpu);
        assert(end_context.regs[REG_AX] == -10);
        assert(end_context.regs[REG_BX] == 40);
        assert(end_context.regs[REG_CX] == 2000);
        assert(end_context.regs[REG_DX] == 285);
        assert(end_context.regs[REG_PC] == 13);
        struct inst *ir_inst = (struct inst *)&end_context.regs[REG_IR];
        assert(ir_inst->op == end_inst.op);
        assert(ir_inst->ra == end_inst.ra);
        assert(ir_inst->imm == end_inst.imm);
    }
    return 0;
}
