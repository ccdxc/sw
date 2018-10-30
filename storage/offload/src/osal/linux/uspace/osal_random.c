/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <stdlib.h>
#include "osal_random.h"

void osal_srand(unsigned int seed)
{
	return srand(seed);
}

int osal_rand(void)
{
	return rand();
}
