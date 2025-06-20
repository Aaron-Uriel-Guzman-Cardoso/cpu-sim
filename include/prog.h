#ifndef PROG_H
#define PROG_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <insts.h>

struct prog {
    char *filename;     /**< Nombre del archivo del programa */
    unsigned long hash; /**< Hash del programa, usado para evitar duplicados */
    size_t length;       /**< Tamaño en instrucciones del programa */
    struct inst instmem[]; /*< Memoria temporal de las instrucciones */
};
struct prog *prog_new(const char *filename);

#endif