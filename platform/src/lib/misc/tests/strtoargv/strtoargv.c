#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>

#include "utils.h"

int
main(int argc, char *argv[])
{
    int i;

    for (i = 1; i < argc; i++) {
        char *av[16];
        int n = strtoargv(strdup(argv[i]), av, 16);
        int j;

        printf("%s:\n", argv[i]);
        for (j = 0; j < n; j++) {
            printf("  %d: %s\n", j, av[j]);
        }
    }

    exit(0);
}
