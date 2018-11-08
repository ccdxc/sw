/*
 * Copyright (c) 2017, Pensando Systems Inc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/param.h>

#include "pal.h"

int
main(int argc, char *argv[])
{
    if(pal_wr_lock(MEMLOCK) != LCK_SUCCESS) {
	printf("FAILED TO GET WR LOCK.\n");
	exit(0);
    }

    if(pal_wr_lock(MEMLOCK) != LCK_SUCCESS) {
        printf("FAILED TO GET WR LOCK AGAIN.\n");
	exit(0);
    }

    while(1);

    if(pal_wr_unlock(MEMLOCK) != LCK_SUCCESS) {
        printf("FAILED TO WR UNLOCK.\n");
	exit(0);
    }

    exit(0);
}

