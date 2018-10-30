/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <sys/random.h>
#include <sys/libkern.h>

void osal_srand(int seed);
int osal_rand(void);

void osal_srand(int seed)
{
	return srandom(seed);
}

int osal_rand(void)
{
	return random();
}
