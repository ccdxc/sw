/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <linux/random.h>

void osal_srand(int seed)
{
	return prandom_seed((unsigned int)seed);
}

int osal_rand(void)
{
	return (int)prandom_u32;
}
