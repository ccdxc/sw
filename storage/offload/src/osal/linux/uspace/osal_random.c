#include <stdlib.h>

void osal_srand(int seed)
{
	return srand(seed);
}

int osal_rand(void)
{
	return rand();
}
