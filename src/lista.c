#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <user_control.h>
#include <prog.h>

#include <msg.h>
#include <cpu.h>
#include <insts.h>
#include <lista.h>
#include <cpu.h>



static int globalPID = 1;

/**
 * \brief Inicializa una lista vacía.
 * \param l Puntero a la lista que se desea inicializar.
 * \return No devuelve ningún valor (void).
 */
Lista *
crearLista() {
    Lista *l = malloc(sizeof(*l));
    if (l == NULL) {
        return NULL;
    }
    l -> inicio = l -> fin = NULL;
    l -> contador = 0;
    return l;
}

/**
 * \brief la tabla de marco de procesos
 */
struct tmp {

};

/**
 * \brief Crea un nuevo bloque de control de proceso (PCB).
 * \param prog El programa asociado al proceso.
 * \param uid El identificador de usuario (UID) del proceso.
 * \return Retorna un puntero al nuevo nodo creado o NULL si falla la asignación de memoria.
 */
PCB *
listaCreaNodo(struct prog *prog, uint8_t uid)
{
    PCB *nuevo_nodo = (PCB*)malloc(sizeof(PCB));
    if (nuevo_nodo) {
        nuevo_nodo->context = (struct cpu_context) { 0 };
        nuevo_nodo->program = prog;
        nuevo_nodo->PID = globalPID++;
        nuevo_nodo->UID = uid;
        nuevo_nodo->P = PBASE;
        nuevo_nodo->KCPU = 0.0;
        nuevo_nodo->sig = NULL;
        /**
         * Dejamos los datos de la TMP como vacíos, pues el diseño del código
         * no nos permite conocer la cantidad de marcos sino hasta que se 
         * llama `swap_allocate_frames`.
         * Nota: puede que este comentario se vuelva impreciso en futuros 
         *       commits
         * TODO: Hacer que la tmp se inicialice aquí, conociendo los marcos 
         *       que se ocupan
         */
        nuevo_nodo->tmp_rows = NULL;
        nuevo_nodo->tmp_size = 0;
    }
    return nuevo_nodo;
}

/**
 * \brief Inserta un nodo al final de la lista.
 * \param l Puntero a la lista donde se insertará el nodo.
 * \param nuevo_nodo Puntero al nodo que se desea insertar.
 * \return No devuelve ningún valor (void).
 */
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

/**
 * \brief Busca un nodo en la lista por su PID.
 * \param l Puntero a la lista donde se realizará la búsqueda.
 * \param PID PID del proceso que se desea buscar.
 * \return Retorna un puntero al nodo encontrado o NULL si no se encuentra.
 */
PCB* listaBuscarPID(Lista *l, int PID) {
    PCB *nodo;
    nodo = l -> inicio;
    while(nodo != NULL && nodo -> PID != PID) {
        nodo = nodo -> sig;
    }
    return nodo;
}


/**
 * \brief Extrae el primer nodo de la lista.
 * \param l Puntero a la lista de donde se extraerá el nodo.
 * \return Retorna un puntero al nodo extraído o NULL si la lista está vacía.
 */
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

/**
 * \brief Busca un nodo en la lista con base en su prioridad (P).
 * \param l Puntero a la lista donde se realizará la búsqueda.
 * \return Retorna un puntero al nodo encontrado o NULL si no se encuentra.
 */
PCB* listaExtraePrioridad(Lista *l) {
    if (l->inicio == NULL) {
        return NULL;
    }

    PCB *actual = l->inicio;
    PCB *anterior = NULL;
    PCB *min_nodo = actual;
    PCB *min_anterior = NULL;

    // Buscar el nodo con menor P
    while (actual != NULL) {
        if (actual->P < min_nodo->P) {
            min_nodo = actual;
            min_anterior = anterior;
        }
        anterior = actual;
        actual = actual->sig;
    }

    // Extraer el nodo con menor P
    if (min_anterior == NULL) {
        // El nodo con menor P es el primero
        l->inicio = min_nodo->sig;
    } else {
        min_anterior->sig = min_nodo->sig;
    }

    if (min_nodo == l->fin) {
        l->fin = min_anterior;
    }

    min_nodo->sig = NULL;
    l->contador--;

    return min_nodo;
}

/**
 * \brief Extrae un nodo de la lista por su PID.
 * \param l Puntero a la lista de donde se extraerá el nodo.
 * \param PID PID del proceso que se desea extraer.
 * \return Retorna un puntero al nodo extraído o NULL si no se encuentra.
 */
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

/**
 * \brief Libera la memoria de un nodo.
 * \param nodo Puntero al nodo que se desea liberar.
 * \return No devuelve ningún valor (void).
 */
void liberarNodo(PCB *nodo){
    free(nodo);
}

/**
 * \brief Libera toda la memoria de una lista.
 * \param l Puntero a la lista que se desea liberar.
 * \return No devuelve ningún valor (void).
 */
void liberarLista(Lista *l){
    PCB *nodo;
    while(l -> inicio != NULL){
        nodo = listaExtraeInicio(l);
        liberarNodo(nodo);
    }
}

/**
 * \brief Convierte la información de un PCB a una cadena de texto.
 * \param pcb Puntero al PCB que se desea convertir.
 * \param str Puntero al buffer donde se almacenará la cadena resultante.
 * \param size Tamaño del buffer.
 * \return No devuelve ningún valor (void).
 */
void 
pcb_as_str(struct PCB *pcb, char *str, size_t size)
{
    if (!pcb || !str || size == 0) {
        return;
    }
    char irstr[50];
    inst_to_str((struct inst *)&pcb->context.regs[REG_IR], irstr, sizeof(irstr));
    User *usr = uc_get_user(pcb->UID);
    snprintf(str, size, "PID: %d, UID: %d, P: %d, KCPU: %.2f, KCPUxU: %.2f,"
             " File: %s, AX: %ld, BX: %ld, CX: %ld, DX: %ld, PC: %ld, IR: %s",
            pcb->PID, pcb->UID, pcb->P, pcb->KCPU,(usr)? usr->KCPUxU: 0.0,
            pcb->program->filename, pcb->context.regs[REG_AX], pcb->context.regs[REG_BX],
            pcb->context.regs[REG_CX], pcb->context.regs[REG_DX], 
            pcb->context.regs[REG_PC], irstr);
}

void 
pcb_as_str_kcpuxu(struct PCB *pcb, char *str, size_t size, float kcpuxu)
{
    if (!pcb || !str || size == 0) {
        return;
    }
    char irstr[50];
    inst_to_str((struct inst *)&pcb->context.regs[REG_IR], irstr, sizeof(irstr));
    snprintf(str, size, "PID: %d, UID: %d, P: %d, KCPU: %.2f, KCPUxU: %.2f, File: %s, AX: %ld, BX: %ld, CX: %ld, DX: %ld, PC: %ld, IR: %s",
    pcb->PID, pcb->UID, pcb->P, pcb->KCPU, kcpuxu, pcb->program->filename, pcb->context.regs[REG_AX], pcb->context.regs[REG_BX],
    pcb->context.regs[REG_CX], pcb->context.regs[REG_DX], pcb->context.regs[REG_PC], irstr);
    
}

/*void insertarName(fileList *names, char *fileName){
    fileTitle *newName = malloc(sizeof(fileTitle));
    newName->fileName = fileName;
    newName->next = NULL;
    if (names->inicio == NULL){
        names->inicio = newName;
        names->fin = newName;
    }
    else{
        names->fin->next = newName;
        names->fin = newName;
    }
}*/