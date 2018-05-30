
/*
 * Copyright (c) 2018, Pensando Systems Inc.
 */

#include "dtls.h"
#if defined(__x86_64__)
#include <readline/readline.h>
#include <readline/history.h>
#endif
#include "parse.h"
#include "cli.h"
#include "clidtls.h"

int cli_ncmds;
struct cli_cmd_entry *cli_cmtab;

static const struct cli_cmd_entry *
cli_find(const char *name)
{
    for (int i = 0; i < cli_ncmds; i++) {
        if (strcmp(cli_cmtab[i].name, name) == 0) {
            return &cli_cmtab[i];
        }
    }
    return NULL;
}

DEFUN(help, 1, "[command]", "Show help")
{
    if (argc == 2) {
        const struct cli_cmd_entry *cm = cli_find(argv[1]);
        if (cm == NULL) {
            printf("%% command not found\n");
            return CMD_ERROR;
        }
        printf("%s - %s\n", cm->name, cm->desc);
        printf("usage: %s %s\n", cm->name, cm->usage);
    } else {
        printf("Commands available:\n");
        for (int i = 0; i < cli_ncmds; i++) {
            if (!(cli_cmtab[i].flags & CLIF_HIDDEN)) {
                printf("  %-15s - %s\n", cli_cmtab[i].name, cli_cmtab[i].desc);
            }
        }
        putchar('\n');
    }
    return CMD_SUCCESS;
}

static void
cli_exec_av(int argc, char *argv[])
{
    const struct cli_cmd_entry *cm = cli_find(argv[0]);
    if (cm == NULL) {
        printf("%% command not found\n");
        return;
    }
    if (argc < cm->minargs) {
        printf("usage: %s %s\n", cm->name, cm->usage);
    } else {
        try {
            if (cm->func(argc, argv) == CMD_USAGE) {
                printf("usage: %s %s\n", cm->name, cm->usage);
            }
        } catch (const std::exception& e) {
            printf("%% command failed with exception\n");
        }
    }
}

static void
cli_procline(char *line)
{
    char *histline = strdup(line);
    char *argv[CLI_MAXARGS + 1];
    int argc;

    for (argc = 0; argc < CLI_MAXARGS; argc++) {
        argv[argc] = strtok(argc ? NULL : line, " \t");
        if (argv[argc] == NULL) {
            break;
        }
    }
    if (argc == 0) {
        free(histline);
        return;
    }
#if defined(__x86_64__)
    add_history(histline);
#endif
    free(histline);
    if (argc == CLI_MAXARGS) {
        printf("%% too many arguments\n");
        return;
    }

    cli_exec_av(argc, argv);
}

static int
cmp_cmtab(const void *a, const void *b)
{
    auto ca = (struct cli_cmd_entry *)a;
    auto cb = (struct cli_cmd_entry *)b;
    return strcmp(ca->name, cb->name);
}

static void
cli_build_commands(void)
{
    extern char __start_cmdtab[];
    extern char __stop_cmdtab[];
    size_t cmsize = __stop_cmdtab - __start_cmdtab;
    cli_ncmds = cmsize / sizeof (struct cli_cmd_entry);
    cli_cmtab = (struct cli_cmd_entry *)malloc(cmsize);
    if (cli_cmtab == NULL) {
        throw std::system_error();
    }
    memcpy(cli_cmtab, __start_cmdtab, cmsize);
    qsort(cli_cmtab, cli_ncmds, sizeof (cli_cmtab[0]), cmp_cmtab);
}

#if !defined(__x86_64__)
#include <iostream>
#include <string>
char *
readline(const char *prompt) {
    static std::string line;
    std::cout << prompt;
    getline(std::cin, line);
    return (char *)line.c_str();
}
#endif

void
cli_run(void)
{
    cli_build_commands();

    if (parse_init() < 0) {
        return;
    }
    cli_generator_init();
    char *s;
    while ((s = readline("> ")) != NULL) {
        if (*s != '\0') {
            cli_procline(s);
        }
    }
}
