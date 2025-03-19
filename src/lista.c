#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include <msg.h>
#include <cpu.h>
#include <insts.h>
#include <lista.h>

static int globalPID = 1;

void crearLista(Lista *l) {
    l -> inicio = l -> fin = NULL;
    l -> contador = 0;
}

PCB *
listaCreaNodo(struct cpu_context context, const char *file_name)
{
    PCB *nuevo_nodo = (PCB*)malloc(sizeof(PCB));
    if (nuevo_nodo) {
        nuevo_nodo->context = context;
        nuevo_nodo->PID = globalPID++;
        strcpy(nuevo_nodo->fileName, file_name);
        nuevo_nodo->programa = fopen(file_name, "r");
        nuevo_nodo->sig = NULL;
    }
    return nuevo_nodo;
}

void listaInsertarFinal(Lista* l, PCB* nuevo_nodo) {
    PCB *ultimo;
    if (l -> inicio == NULL) {
        l -> inicio = l -> fin = nuevo_nodo;
	    (l -> contador)++;
    } else {
        l -> fin = nuevo_nodo;
        ultimo = l -> inicio;
        while(ultimo -> sig != NULL) {
            ultimo = ultimo -> sig;
        }
        ultimo -> sig = l->fin;
        (l -> contador)++;
    }
}

PCB* listaBuscarPID(Lista *l, int PID) {
    PCB *nodo;
    nodo = l -> inicio;
    while(nodo != NULL && nodo -> PID != PID) {
        nodo = nodo -> sig;
    }
    return nodo;
}

PCB* listaExtraeInicio(Lista *l) {
    PCB *nodo;
    if(l -> inicio != NULL){
		nodo = l -> inicio;
		if(l -> inicio == l -> fin) // Sólo hay un elemento.
			l -> fin = l -> fin -> sig;
		l -> inicio = l -> inicio -> sig;
		nodo -> sig = NULL;
		(l -> contador)--;
		
		return nodo;
	}
	else
		return NULL;
}

PCB* listaExtraePID(Lista *l, int PID) {
    PCB *nodo, *anterior;
    if(l -> inicio != NULL){
        nodo = l -> inicio;
        anterior = NULL;
        while(nodo != NULL && nodo -> PID != PID){
            anterior = nodo;
            nodo = nodo -> sig;
        }
        if(nodo != NULL){
            if(anterior == NULL){ // Se elimina el primer elemento.
                l -> inicio = l -> inicio -> sig;
                nodo -> sig = NULL;
            }
            else{
                anterior -> sig = nodo -> sig;
                nodo -> sig = NULL;
            }
            if(nodo == l -> fin){ // Se elimina el último elemento.
                l -> fin = anterior;
                nodo -> sig = NULL;
            }
            (l -> contador)--;
        }
        return nodo;
    }
    else
        return NULL;
}

void liberaNodo(PCB *nodo){
    fclose(nodo -> programa);
    free(nodo);
}

void liberarLista(Lista *l){
    PCB *nodo;
    while(l -> inicio != NULL){
        nodo = listaExtraeInicio(l);
        liberaNodo(nodo);
    }
}

/**
 * \brief Convierte estructura PCB a una cadena legible por seres humanos
 */
void 
pcb_as_str(struct PCB *self, char *str, size_t size)
{
    if (!self || !str || size == 0) {
        return;
    }
    char irstr[50];
    inst_to_str((struct inst *)&self->context.regs[REG_IR], irstr, sizeof(irstr));
    snprintf(str, size, "PID: %d, File: %s, AX: %d, BX: %d, CX: %d, DX: %d, PC: %d, IR: %s",
             self->PID, self->fileName, self->context.regs[REG_AX], self->context.regs[REG_BX],
             self->context.regs[REG_CX], self->context.regs[REG_DX], self->context.regs[REG_PC], irstr);
}