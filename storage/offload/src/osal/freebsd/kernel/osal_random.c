/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <sys/random.h>
#include <sys/libkern.h>
#include "osal_random.h"

void osal_srand(unsigned int seed)
{
	return srandom(seed);
}

int osal_rand(void)
{
	return random();
}
