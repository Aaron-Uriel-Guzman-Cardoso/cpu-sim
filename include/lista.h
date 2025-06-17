#ifndef LISTA_H
#define LISTA_H

#include <stdint.h>
#include <stdio.h>
#include <cpu.h>

#define PBASE 60

typedef struct fileTitle {
    char *fileName;
    struct fileList *next;
}fileTitle;

typedef struct fileList {
    struct fileTitle *inicio;
    struct fileTitle *fin;
}fileList;

typedef struct PCB {
    struct cpu_context context;
    uint32_t PID;
    char fileName[256];
    FILE *programa;
    struct PCB *sig;
    uint8_t UID;
    int P;
    float KCPU;
    uint16_t program_size;                   // Total de instrucciones del programa
    uint16_t tmp_size;
    struct {int32_t on_swap; int32_t on_ram} *tmp_rows;       // Datos de la tabla de marcos del proceso (TMP)
} PCB;

/**
 * TODO: Implementar una lista genérica que no requiera de PCB como nodo.
 */
typedef struct Cabecera {
    PCB *inicio;
    PCB *fin;
    unsigned contador;
} Lista;

void crearLista(Lista *);
PCB *listaCreaNodo(struct cpu_context context, const char *file_name, uint8_t uid);
void listaInsertarFinal(Lista *, PCB *);
PCB* listaBuscarPID(Lista *, int);
PCB* listaExtraeInicio(Lista *);
PCB* listaExtraePrioridad(Lista *);
PCB* listaExtraePID(Lista *, int);
void liberarNodo(PCB *);
void liberarLista(Lista *);
void pcb_as_str(struct PCB *cpu, char *str, size_t size);
void pcb_as_str_kcpuxu(struct PCB *cpu, char *str, size_t size, float kcpuxu);

#endif // LISTA_H
