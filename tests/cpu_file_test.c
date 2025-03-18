#include "insts.h"
#include <cpu.h>
#include <assert.h>
#include <string.h>

/*
 * Cómo ejecutar esta prueba sin problema
 * Para ejecutar esta prueba será necesario hacer cd al directorio de samples,
 * así es como se detectarán los archivos de prueba.
 */

int32_t
main(void)
{
    struct cpu *cpu = cpu_new();
    cpu_load_insts_from_file(cpu, "PROG");
    cpu_set_freq(cpu, 1E3);
    cpu_sync(cpu);
    {
        struct cpu_context end_context = cpu_dump_context(cpu);
        const struct inst end_inst = {
            .op = OP_END,
            .ra = REG_AX,
            .imm = 0
        };
        assert(end_context.regs[REG_AX] == 0);
        assert(end_context.regs[REG_BX] == 30);
        assert(end_context.regs[REG_CX] == 0);
        assert(end_context.regs[REG_DX] == 0);
        assert(end_context.regs[REG_PC] == 9);
        assert(memcmp(
            &end_inst,
            &end_context.regs[REG_IR],
            sizeof(end_inst)) == 0
        );
    }
    return 0;
}
