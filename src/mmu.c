#include <stdint.h>
#include <stdio.h>
#include <insts.h>
#include <os.h>
#include <lista.h>
#include <swap.h>
#include <assert.h>
#include <ncurses.h>

struct inst mmu_get_inst(uint16_t addr);

/**
 * \brief Obtiene la siguiente instrucción a usar por la CPU en base
 *        a la dirección virtual recibida.
 * \param addr Dirección virtual de la siguiente instrucción
 *             (usalmente el registro PC) 
 * \return La instrucción encontrada en la dirección física calculada,
 *         MOV AX 0 si está inicializado en ceros.
 */
struct inst
mmu_get_inst(uint16_t addr)
{
    const uint32_t pid = os_get_curr_pid();
    assert(pid);
    const struct PCB *curr_proc = os_get_proc(pid);
    const uint16_t relative_target_frame = addr / FRAME_SIZE;
    const uint16_t offset = addr % FRAME_SIZE;
    const uint16_t abs_target_frame = curr_proc->tmp[relative_target_frame];
    return swap_get(abs_target_frame, offset);
};
