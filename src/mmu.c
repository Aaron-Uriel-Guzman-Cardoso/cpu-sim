#include <stdint.h>
#include <stdio.h>
#include <insts.h>
#include <os.h>
#include <lista.h>
#include <swap.h>
#include <assert.h>
#include <ncurses.h>
#include <swap.h>
#include <msg.h>

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
    struct PCB *curr_proc = os_get_proc(pid);
    const uint16_t relative_target_frame = virt_addr / SWAP_PAGE_SIZE;
    const uint16_t offset = virt_addr % SWAP_PAGE_SIZE;
    /* 
     * Verificamos que la dirección a acceder está entre las permitidas para el
     * proceso, en caso de salirnos, regresamos END.
     */
    if (relative_target_frame >= curr_proc->tmp_size ||
        (relative_target_frame == (curr_proc->tmp_size - 1) &&
         (curr_proc->program->length % SWAP_PAGE_SIZE) < offset))
    {
        return (struct inst) { .op = OP_END, .ra = REG_AX, .imm = 0 };
    }

    //const uint16_t abs_target_frame 
    int32_t abs_target_frame
        = curr_proc->tmp_rows[relative_target_frame].on_ram;
    if (abs_target_frame == -1) {
        msg_log(LOG_LEVEL_WARN, "Se emite un fallo de página");
        os_page_fault_handler(curr_proc, relative_target_frame);
        abs_target_frame  = curr_proc->tmp_rows[relative_target_frame].on_ram; 
    }
    assert((abs_target_frame != -1) && "Al parecer el marco de la página no está"
           " en RAM tras dos intentos (que deberían ser suficientes).");

    const uint16_t abs_byte_addr 
        = (abs_target_frame * SWAP_PAGE_SIZE + offset) * WORD_SIZE;
    static uint32_t last_pid = 0, last_accessed_frame = RAM_FRAME_SIZE + 1;
    /** 
     * Activamos el bit de utilización antes de realmente interactuar con RAM,
     * obtenemos la palabra en RAM y la convertimos a una instrucción legible
     * por la CPU.
     */
    tmm_touch(abs_target_frame);
    word_t word = ram_read_word(abs_byte_addr);
    return *((struct inst *)(&word));
}