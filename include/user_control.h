#ifndef USER_CONTROL_H
#define USER_CONTROL_H

#include <stdbool.h>
#include <stdint.h>

#define MAX_USER_STATS 256

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

void uc_init(void);
bool uc_user_exists(uint8_t uid);
User *uc_get_user(uint8_t uid);
bool uc_alloc_user(User *user);
int32_t uc_get_current_users(void);
void uc_dealloc_user(uint8_t uid);
void update_user_stats(uint8_t uid, float value);
float get_user_stats(uint8_t uid);
double uc_get_weight(void);
User *crearUsuario(uint8_t uid);



#endif