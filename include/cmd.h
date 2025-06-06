#ifndef CMD_H
#define CMD_H

#define MAX_CMD_CHARS 10

struct cmd {
    char name[MAX_CMD_CHARS];
    char arg1[MAX_CMD_CHARS];
    char arg2[MAX_CMD_CHARS];
};

struct cmd *cmd_decode(const char *buf);

#endif