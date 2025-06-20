#ifndef LISTA_H
#define LISTA_H

#include <stdint.h>
#include <stdio.h>
#include <cpu.h>
#include <prog.h>


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
    struct prog *program;               // Programa asociado al proceso
    struct PCB *sig;
    uint8_t UID;
    int P;
    float KCPU;
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

Lista *crearLista();
PCB *listaCreaNodo(struct prog *prog, uint8_t uid);
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
