#ifndef LISTA_H
#define LISTA_H

#include <stdio.h>
#include <cpu.h>

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
    struct inst instmem[128];
    struct PCB *sig;
    int UID;
    int P;
    float KCPU;
    float KCPUxU;
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
PCB *listaCreaNodo(struct cpu_context context, const char *file_name, int uid);
void listaInsertarFinal(Lista *, PCB *);
PCB* listaBuscarPID(Lista *, int);
PCB* listaExtraeInicio(Lista *);
PCB* listaExtraePID(Lista *, int);
void liberarNodo(PCB *);
void liberarLista(Lista *);
void pcb_as_str(struct PCB *self, char *str, size_t size);

#endif // LISTA_H