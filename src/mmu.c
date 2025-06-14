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
 * \brief Obtiene la siguiente instrucción a usar por la CPU en base a la 
 *        dirección virtual recibida.
 * \param addr Dirección virtual de la siguiente instrucción
 *             (usalmente tomada del registro PC)
 * \return La instrucción encontrada en la dirección física calculada,
 *         END en caso de llegar al final del programa o ingresar una
 *         dirección inválida.
 */
struct inst
mmu_get_inst(const uint16_t virt_addr)
{
    const uint32_t pid = os_get_curr_pid();
    assert(pid);
    const struct PCB *curr_proc = os_get_proc(pid);
    const uint16_t relative_target_frame = virt_addr / SWAP_PAGE_SIZE;
    const uint16_t offset = virt_addr % SWAP_PAGE_SIZE;
    /* 
     * Verificamos que la dirección a acceder está entre las permitidas para el
     * proceso, en caso de salirnos, regresamos END.
     */
    if (relative_target_frame >= curr_proc->tmp_size ||
        (relative_target_frame == (curr_proc->tmp_size - 1) &&
         (curr_proc->program_size % SWAP_PAGE_SIZE) < offset))
    {
        return (struct inst) { .op = OP_END, .ra = REG_AX, .imm = 0 };
    }
    const uint16_t abs_target_page = curr_proc->tmp[relative_target_frame];
    const uint16_t abs_byte_addr = abs_target_page * SWAP_PAGE_SIZE * WORD_SIZE;
    return ram_read(abs_byte_addr, WORD_SIZE);
};
