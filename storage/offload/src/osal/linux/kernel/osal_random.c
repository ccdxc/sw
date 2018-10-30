/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <linux/random.h>
#include "osal_random.h"

void osal_srand(unsigned int seed)
{
	return prandom_seed(seed);
}

int osal_rand(void)
{
	return (int)prandom_u32();
}
