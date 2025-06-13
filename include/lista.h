#ifndef LISTA_H
#define LISTA_H

#include <stdint.h>
#include <stdio.h>
#include <cpu.h>

#define MAX_USER_STATS 256
#define PBASE 60

typedef struct fileTitle {
    char *fileName;
    struct fileList *next;
}fileTitle;

typedef struct fileList {
    struct fileTitle *inicio;
    struct fileTitle *fin;
}fileList;

/**
 * Guarda uno de los índices de página de un proceso tanto en swap como en RAM
 * 
 * Nota: son int32_t porque queremos usar valores negativos para indicar que
 *       tal marco no tiene presencia en alguna de las memorias.
 */
struct page_table_entry {
    int32_t on_swap;
    int32_t on_ram;
};

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
    /** 
     * TODO: adecuar el código que crea a PCB para crear a tmp como un flexible
     *       array member y que el tamaño se guarde como constante.
     */
    const uint16_t tmp_size;                   // Tamaño de TMP (marcos usados)
    struct page_table_entry *tmp;       // Datos de la tabla de marcos del proceso (TMP)
} PCB;


typedef struct User {
    uint8_t uid;
    float KCPUxU;
    uint8_t process_counter;
} User;

typedef struct UserStats {
    uint8_t uid;
    float KCPUxU;
} UserStats;

extern UserStats user_stats[MAX_USER_STATS];
extern int user_stats_count;

struct user_control {
    uint8_t current_users;
    User *users[256];
};

bool uc_user_exists(struct user_control *self, uint8_t uid);
User *uc_get_user(struct user_control *self, uint8_t uid);
bool uc_alloc_user(struct user_control *self, User *user);
double uc_get_weight(struct user_control *self);
void uc_dealloc_user(struct user_control *self, uint8_t uid);
void update_user_stats(uint8_t uid, float value);
float get_user_stats(uint8_t uid);	

//void update_user_stats(uint8_t uid, float KCPUxU, UserStats *user_stats, int user_stats_count);
//float get_user_stats(uint8_t uid, UserStats *user_stats, int user_stats_count);

/**
 * \brief Coloca el usuario recién creado en el arreglo de usuarios
 * \return true si no hay espacio y no se puede meter el usuario
 */


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
void pcb_as_str(struct PCB *self, char *str, size_t size, User *user);
void pcb_as_str_kcpuxu(struct PCB *self, char *str, size_t size, float kcpuxu);
bool uc_alloc_user(struct user_control *self, User *user);
User *uc_get_user(struct user_control *self, uint8_t uid);
bool uc_user_exists(struct user_control *self, uint8_t uid);
double uc_get_weight(struct user_control *self);
User *crearUsuario(uint8_t uid);

#endif // LISTA_H
