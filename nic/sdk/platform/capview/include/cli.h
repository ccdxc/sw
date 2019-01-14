
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#ifndef __CLI_H__
#define __CLI_H__

#define CLI_MAXARGS     16
void cli_run(void);

enum cmd_res_e {
    CMD_SUCCESS = 0,
    CMD_USAGE = -1,
    CMD_FAILED = -2,
    CMD_ERROR = -3,
};

struct cli_cmd_entry {
    const char *name;
    int minargs;
    const char *usage;
    const char *desc;
    uint32_t flags;
    cmd_res_e (*func)(int argc, char *argv[]);
};
#define CLIF_HIDDEN     0x1

#define _DEFUN(n, m, u, d, f) \
    static cmd_res_e cm_##n(int argc, char *argv[]); \
    static struct cli_cmd_entry cme_##n \
        __attribute__((aligned(sizeof (void *)), section("cmdtab"), used)) = \
        { #n, m, u, d, f, cm_##n }; \
    static cmd_res_e cm_##n(int argc, char *argv[])

#define DEFUN(n, m, u, d)   _DEFUN(n, m, u, d, 0)
#define DEFUN_H(n, m, u, d) _DEFUN(n, m, u, d, CLIF_HIDDEN)

#endif
