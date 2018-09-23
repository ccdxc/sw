/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __CMD_H__
#define __CMD_H__

typedef struct cmd_s {
    const char *name;
    const char *help;
    void (*func)(int argc, char *argv[]);
} cmd_t;

#define CMDFUNC(fnc, hlp) \
static cmd_t cmd_t_##fnc \
    __attribute__((used, section("cmdtab"), aligned(4))) = \
{ \
    .name = #fnc, \
    .help = hlp, \
    .func = fnc, \
}

#endif /* __CMD_H__ */
