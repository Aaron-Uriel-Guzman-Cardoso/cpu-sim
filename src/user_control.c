#include <stdlib.h>

#include <user_control.h>
#include <msg.h>

struct user_control *uc;
UserStats user_stats[MAX_USER_STATS];

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
        user->process_counter = 0;
    }
    return user;
}

void
uc_init(void)
{
    uc = malloc(sizeof(*uc));
    if (!uc) {
        msg_log(LOG_LEVEL_ERROR, "Error al inicializar user_control.\n");
        exit(1);
    }
    uc->current_users = 0;
    for (int i = 0; i < 256; i++) {
        uc->users[i] = NULL;
    }
}

/**
 * \brief Verifica si un usuario existe en el arreglo de usuarios.
 * \param uid UID del usuario que se desea verificar.
 * \return Retorna true si el usuario existe, false en caso contrario.
 */
bool 
uc_user_exists(uint8_t uid)
{
    if(uc->current_users == 0) {
        return false;
    }
    for (int i = 0; i < 256; i++) {
        if (uc->users[i]) {
            if (uc->users[i]->uid == uid) {
                return true;
            }
        }
    }
    return false;
}

/**
 * \brief Busca un usuario en el arreglo de usuarios por su UID.
 * \param uid UID del usuario que se desea buscar.
 * \return Retorna un puntero al usuario encontrado o NULL si no se encuentra.
 */
User *
uc_get_user(uint8_t uid)
{
    if(uc->current_users == 0) {
        return NULL;
    }
    for (int i = 0; i < 256; i++) {
        if (uc->users[i]) {
            if (uc->users[i]->uid == uid) {
                return uc->users[i];
            }
        }
    }
    return NULL;
}

/**
 * \brief Asigna un usuario al arreglo de usuarios.
 * \param user Puntero al usuario que se desea asignar.
 * \return Retorna true si no hay espacio y no se puede asignar el usuario.
 */
bool
uc_alloc_user(User *user)
{
    if (uc->current_users == 256) {
        return true;
    }
    for (uint8_t i = 0; i < 256; i += 1) {
        if (uc->users[i] == NULL) {
            uc->users[i] = user;
            uc->current_users += 1;
            return false;
        }
    }
    return true;
}

/**
 * \brief Actualiza las estadísticas de un usuario.
 * \param uid UID del usuario que se desea actualizar.
 * \param value Valor a asignar a KCPUxU.
 * \return No devuelve ningún valor (void).
 */
void
update_user_stats(uint8_t uid, float value)
{
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

/**
 * \brief Obtiene las estadísticas de un usuario.
 * \param uid UID del usuario que se desea obtener.
 * \return Retorna el valor de KCPUxU del usuario o 0.0 si no se encuentra.
 */
float
get_user_stats(uint8_t uid)
{
    for (int i = 0; i < user_stats_count; i++) {
        if (user_stats[i].uid == uid) {
            return user_stats[i].KCPUxU;
        }
    }
    return 0.0f;
} 

/**
 * \brief Calcula el peso de un usuario en función de la cantidad de usuarios activos.
 * \return Retorna el peso del usuario.
 */
double uc_get_weight(void) {
    return (uc->current_users)? 1.0/uc->current_users: 0.0;
}


/**
 * \brief Libera el espacio de un usuario, quitándolo de los usuarios activos
 * \param uid UID del usuario que se desea liberar.
 * \return No devuelve ningún valor (void).
 */
void
uc_dealloc_user(uint8_t uid)
{
    for (int i = 0; i < 256; i++) {
        if (uc->users[i] && uc->users[i]->uid == uid) {
            free(uc->users[i]);
            uc->users[i] = NULL;
            uc->current_users -= 1;
            break;
        }
    }
}

int32_t
uc_get_current_users(void)
{
    return uc->current_users;
}