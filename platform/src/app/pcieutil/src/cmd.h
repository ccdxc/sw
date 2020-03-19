/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#ifndef __CMD_H__
#define __CMD_H__

typedef struct cmd_s {
    const char *name;
    const char *desc;
    const char *usage;
    void (*func)(int argc, char *argv[]);
} cmd_t;

#define CMDFUNC(F, D, U) \
static cmd_t cmd_t_##F \
    __attribute__((used, section("cmdtab"), aligned(4))) = \
{ \
    .name = #F, \
    .desc = D, \
    .usage = U, \
    .func = F, \
}

int cmd_run(int argc, char *argv[]);
int cmd_runstr(char *cmdstr);

#endif /* __CMD_H__ */
