#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int
main(int argc, const char *argv[])
{
    int mem = 0;
    int delay = 0;
    int total = 0;
    int configured_total = 0;
    if (argc != 4) {
        printf("usage: %s <Block size>MB <Max size>MB <Sleep duration>s",
               argv[0]);
        return -1;
    }

    mem = atoi(argv[1]);
    configured_total = atoi(argv[2]);
    delay = atoi(argv[3]);

   
    while (configured_total > total) {
        size_t size = mem * (1024 * 1204);
        char *c = (char *)malloc(size);
        memset(c, 0xAA, size);
        total += mem;
        printf("Allocated: %i\n", total);
        sleep(delay);
    }

    printf("Sleeping for ever\n");
    while (true) {
        sleep(100);
    }

    return 0;
}
