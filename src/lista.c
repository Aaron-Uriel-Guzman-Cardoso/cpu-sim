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

#define MAX_USER_STATS 256

static int globalPID = 1;

/**
 * \brief Inicializa una lista vacía.
 * \param l Puntero a la lista que se desea inicializar.
 * \return No devuelve ningún valor (void).
 */
void crearLista(Lista *l) {
    l -> inicio = l -> fin = NULL;
    l -> contador = 0;
}

/**
 * \brief Crea un nuevo nodo de tipo PCB.
 * \param context Contexto de la CPU para el nuevo nodo.
 * \param file_name Nombre del archivo asociado al proceso.
 * \return Retorna un puntero al nuevo nodo creado o NULL si falla la asignación de memoria.
 */
PCB 
*listaCreaNodo(struct cpu_context context, const char *file_name, uint8_t uid) {
    PCB *nuevo_nodo = (PCB*)malloc(sizeof(PCB));
    if (nuevo_nodo) {
        nuevo_nodo->context = context;
        nuevo_nodo->programa = fopen(file_name, "r");
        if (nuevo_nodo->programa != NULL) {
            // Solo incrementar el PID si el archivo se abre correctamente
            nuevo_nodo->PID = globalPID++;
            strcpy(nuevo_nodo->fileName, file_name);
            nuevo_nodo->sig = NULL;
            nuevo_nodo->UID = uid;
            nuevo_nodo->P = 0; // Inicializar P a 0
            nuevo_nodo->KCPU = 0.0; // Inicializar KCPU a 0.0
        } else {
            // Si el archivo no se puede abrir, liberar el nodo y retornar NULL
            free(nuevo_nodo);
            nuevo_nodo = NULL;
        }
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
 * \brief Verifica si un usuario existe en el arreglo de usuarios.
 * \param self Puntero al control de usuarios.
 * \param uid UID del usuario que se desea verificar.
 * \return Retorna true si el usuario existe, false en caso contrario.
 */
bool uc_user_exists(struct user_control *self, uint8_t uid) {
    if(self->current_users == 0) {
        return false;
    }
    for (int i = 0; i < 256; i++) {
        if (self->users[i]) {
            if (self->users[i]->uid == uid) {
                return true;
            }
        }
    }
    return false;
}


/**
 * \brief Crea un nuevo usuario.
 * \param uid UID del nuevo usuario.
 * \return Retorna un puntero al nuevo usuario creado o NULL si falla la asignación de memoria.
 */
User *crearUsuario(uint8_t uid) {
    User *user = malloc(sizeof(User));
    if (user) {
        user->uid = uid;
        user->KCPUxU = 0.0;
    }
    return user;
}

/**
 * \brief Busca un usuario en el arreglo de usuarios por su UID.
 * \param self Puntero al control de usuarios.
 * \param uid UID del usuario que se desea buscar.
 * \return Retorna un puntero al usuario encontrado o NULL si no se encuentra.
 */

User *uc_get_user(struct user_control *self, uint8_t uid) {
    if(self->current_users == 0) {
        return NULL;
    }
    for (int i = 0; i < 256; i++) {
        if (self->users[i]) {
            if (self->users[i]->uid == uid) {
                return self->users[i];
            }
        }
    }
    return NULL;
}

/**
 * \brief Asigna un usuario al arreglo de usuarios.
 * \param self Puntero al control de usuarios.
 * \param user Puntero al usuario que se desea asignar.
 * \return Retorna true si no hay espacio y no se puede asignar el usuario.
 */
bool
uc_alloc_user(struct user_control *self, User *user)
{
    if (self->current_users == 256) {
        return true;
    }
    for (uint8_t i = 0; i < 256; i += 1) {
        if (self->users[i] == NULL) {
            self->users[i] = user;
            self->current_users += 1;
            return false;
        }
    }
    return true;
}

void update_user_stats(uint8_t uid, float value) {
    for (int i = 0; i < user_stats_count; i++) {
        if (user_stats[i].uid == uid) {
            user_stats[i].KCPUxU = value;
            return;
        }
    }
    // Si no existe, agregar nuevo
    if (user_stats_count < MAX_USER_STATS) {
        user_stats[user_stats_count].uid = uid;
        user_stats[user_stats_count].KCPUxU = value;
        user_stats_count++;
    }
}

float get_user_stats(uint8_t uid) {
    for (int i = 0; i < user_stats_count; i++) {
        if (user_stats[i].uid == uid) {
            return user_stats[i].KCPUxU;
        }
    }
    return 0.0f;
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
    fclose(nodo -> programa);
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
 * \param self Puntero al PCB que se desea convertir.
 * \param str Puntero al buffer donde se almacenará la cadena resultante.
 * \param size Tamaño del buffer.
 * \return No devuelve ningún valor (void).
 */
void 
pcb_as_str(struct PCB *self, char *str, size_t size, User *user)
{
    if (!self || !str || size == 0) {
        return;
    }
    char irstr[50];
    inst_to_str((struct inst *)&self->context.regs[REG_IR], irstr, sizeof(irstr));
    if (user) {
        snprintf(str, size, "PID: %d, UID: %d, P: %d, KCPU: %.2f, KCPUxU: %.2f, File: %s, AX: %ld, BX: %ld, CX: %ld, DX: %ld, PC: %ld, IR: %s",
            self->PID, self->UID, self->P, self->KCPU, user->KCPUxU, self->fileName, self->context.regs[REG_AX], self->context.regs[REG_BX],
            self->context.regs[REG_CX], self->context.regs[REG_DX], self->context.regs[REG_PC], irstr);
    } else {
        snprintf(str, size, "PID: %d, UID: %d, P: %d, KCPU: %.2f, KCPUxU: %.2f, File: %s, AX: %ld, BX: %ld, CX: %ld, DX: %ld, PC: %ld, IR: %s",
            self->PID, self->UID, self->P, self->KCPU, 0.0, self->fileName, self->context.regs[REG_AX], self->context.regs[REG_BX],
            self->context.regs[REG_CX], self->context.regs[REG_DX], self->context.regs[REG_PC], irstr);
    }
    
}

void pcb_as_str_kcpuxu(struct PCB *self, char *str, size_t size, float KCPUxU) {
    if (!self || !str || size == 0) {
        return;
    }
    char irstr[50];
    inst_to_str((struct inst *)&self->context.regs[REG_IR], irstr, sizeof(irstr));
    snprintf(str, size, "PID: %d, UID: %d, P: %d, KCPU: %.2f, KCPUxU: %.2f, File: %s, AX: %ld, BX: %ld, CX: %ld, DX: %ld, PC: %ld, IR: %s",
        self->PID, self->UID, self->P, self->KCPU, KCPUxU, self->fileName, self->context.regs[REG_AX], self->context.regs[REG_BX],
        self->context.regs[REG_CX], self->context.regs[REG_DX], self->context.regs[REG_PC], irstr);
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

/**
 * \brief Calcula el peso de un usuario en función de la cantidad de usuarios activos.
 * \param self Puntero al control de usuarios.
 * \return Retorna el peso del usuario.
 */
double uc_get_weight(struct user_control *self) {
    return (self->current_users)? 1.0/self->current_users: 1.0;
}


/**
 * \brief Libera el espacio de un usuario, quitándolo de los usuarios activos
 * \param self Puntero al control de usuarios.
 * \param uid UID del usuario que se desea liberar.
 * \return No devuelve ningún valor (void).
 */
void
uc_dealloc_user(struct user_control *self, uint8_t uid)
{
    for (int i = 0; i < 256; i++) {
        if (self->users[i] && self->users[i]->uid == uid) {
            free(self->users[i]);
            self->users[i] = NULL;
            self->current_users -= 1;
            break;
        }
    }
}
