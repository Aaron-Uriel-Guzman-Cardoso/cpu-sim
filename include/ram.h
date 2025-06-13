#ifndef RAM_H
#define RAM_H
#include <stdint.h>

void ram_init();
void *ram_read(uint16_t address, uint16_t size);
void ram_write(uint16_t address, const void *data, uint16_t size); // Siento que esta debería ser privada

#endif