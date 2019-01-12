#ifndef FAKE_LINUX_SLAB_H
#define FAKE_LINUX_SLAB_H

#include <errno.h>
#include <stdlib.h>

#define GFP_KERNEL 0

static inline void *kmalloc(size_t size, int gfp)
{
	return calloc(1, size);
}

static inline void *kzalloc(size_t size, int gfp)
{
	return calloc(1, size);
}

static inline void *kcalloc(size_t nmemb, size_t size, int gfp)
{
	return calloc(nmemb, size);
}

static inline void kfree(void *ptr)
{
	return free(ptr);
}

#endif
