#ifndef LISTA_H
#define LISTA_H

#include <stdio.h>

typedef struct PCB {
    int PID;
    int AX;
    int BX;
    int CX;
    int DX;
    int PC;
    char IR[100];
    char fileName[256];
    FILE *programa;
    struct PCB *sig;
} PCB;

typedef struct Cabecera {
    PCB *inicio;
    PCB *fin;
    unsigned contador;
} Lista;

void crearLista(PCB *);
PCB* listaCreaNodo(int ax, int bx, int cx, int dx, int pc, const char *ir, const char *file_name);
void listaInsertarFinal(Lista *, PCB *);

#endif // LISTA_H