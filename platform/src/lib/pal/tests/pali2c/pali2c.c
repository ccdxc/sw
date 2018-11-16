/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#include "platform/src/lib/pal/include/pal.h"

#define QSFP_READ_SIZE 10

int main(int argc, char *argv[])
{
    const uint8_t *fru_buffer = malloc(FRU_SIZE);

    int i = 0;

    if (fru_buffer == NULL) {
        printf("failed to allocate memory.\n");
        return -1;
    }
    memset((uint8_t*)fru_buffer, 0, FRU_SIZE);

    pal_fru_read(fru_buffer, FRU_SIZE, 1);

    printf("after read\n");
    for (i = 0; i < FRU_SIZE; i++)
        printf("%x", fru_buffer[i]);
    printf("\nreading done\n");

    return 0;
}

