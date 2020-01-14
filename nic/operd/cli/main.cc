#include <stdio.h>
#include <string.h>

#include "operdctl.hpp"

struct cmd_handler_ {
    const char *name;
    cmd_handler_fn fn;
};

static struct cmd_handler_ g_cmd_handlers[] = {
    {"dump", dump},
    {"level", level},
};

static int g_cmd_handlers_len =
    sizeof(g_cmd_handlers) / sizeof(g_cmd_handlers[0]);

int
main (int argc, const char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <command> <command options>\n", argv[0]);
        printf("Available comands:\n");
        for (int i = 0; i < g_cmd_handlers_len; i++) {
            printf("%s ", g_cmd_handlers[i].name);
        }
        printf("\n");
        return -1;
    }

    for (int i = 0; i < g_cmd_handlers_len; i++) {
        if (strcasecmp(g_cmd_handlers[i].name, argv[1]) == 0) {
            return g_cmd_handlers[i].fn(argc - 2, &argv[2]);
        }
    }

    printf("Unknown command: %s\n", argv[1]);
}
