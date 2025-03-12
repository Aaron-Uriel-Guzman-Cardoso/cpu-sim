#include <cpu.h>
#include <assert.h>

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
    for (int32_t i = 0; i < 8; i += 1) {
        cpu_next_cycle(cpu);
        assert(cpu->state == CPU_READY);
    }
    assert(cpu->regs[REG_AX] == 0);
    assert(cpu->regs[REG_BX] == 0);
    assert(cpu_next_cycle(cpu) == 3);
    return 0;
}