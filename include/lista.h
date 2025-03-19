#ifndef LISTA_H
#define LISTA_H

#include <stdio.h>
#include <cpu.h>


typedef struct PCB {
    struct cpu_context context;
    int PID;
    char fileName[256];
    FILE *programa;
    struct PCB *sig;
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
PCB *listaCreaNodo(struct cpu_context context, const char *file_name);
void listaInsertarFinal(Lista *, PCB *);
PCB* listaBuscarPID(Lista *, int);
PCB* listaExtraeInicio(Lista *);
PCB* listaExtraePID(Lista *, int);
void liberarNodo(PCB *);
void liberarLista(Lista *);
void pcb_as_str(struct PCB *self, char *str, size_t size);

#endif // LISTA_H