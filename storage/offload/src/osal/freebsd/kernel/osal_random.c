#include <sys/random.h>
#include <sys/libkern.h>

void osal_srand(int seed)
{
	return srandom(seed);
}

int osal_rand(void)
{
	return random();
}
