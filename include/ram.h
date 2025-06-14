#ifndef RAM_H
#define RAM_H
#include <stdint.h>
#include <cpu.h>
#include <swap.h>

#define RAM_NUM_FRAMES 16
#define RAM_FRAME_SIZE SWAP_PAGE_SIZE
#define RAM_TOTAL_WORD_SIZE (RAM_NUM_FRAMES * RAM_FRAME_SIZE) // Total de palabras en RAM
#define RAM_TOTAL_BYTE_SIZE RAM_TOTAL_WORD_SIZE * WORD_SIZE   // Tamaño total de RAM en bytes

void ram_init(void);
word_t ram_read_word(const uint16_t addr);
int32_t ram_read(void *dst, const uint16_t addr, const uint16_t size);

/* ¿Deberían ser funciones privadas? */
int32_t ram_write_word(const uint16_t addr, const word_t word);
int32_t ram_write(const uint16_t addr, const void *data, const uint16_t size);

uint16_t ram_frame_to_addr(uint16_t frame_num);

#endif