/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "pal.h"

#define SUCCESS 0
#define FAIL -1
#define QSFP_READ_SIZE 10

int main(int argc, char *argv[])
{
    u_int8_t *fru_buffer = NULL;
    u_int8_t *qsfp_buffer = NULL;
    fru_buffer = malloc(FRU_SIZE);
    int i = 0;

    if (fru_buffer == NULL) {
        printf("failed to allocate memory.\n");
        return FAIL;
    }
    memset(fru_buffer, 0, FRU_SIZE);

    pal_fru_read(fru_buffer, FRU_SIZE, 1);

    printf("after read\n");
    for (i = 0; i < FRU_SIZE; i++)
        printf("%x", fru_buffer[i]);
    printf("\nreading done\n");

    // test qsfp read.
    qsfp_buffer = malloc(QSFP_READ_SIZE);
    if (qsfp_buffer == NULL) {
        printf("failed to allocate memory.\n");
        return FAIL;
    }
    memset(qsfp_buffer, 0, QSFP_READ_SIZE);

    pal_qsfp_read(qsfp_buffer, QSFP_READ_SIZE, 1, 1);

    printf("after read\n");
    for (i = 0; i < QSFP_READ_SIZE; i++)
        printf("%x", qsfp_buffer[i]);
    printf("\nreading done\n");


    return SUCCESS;
}

