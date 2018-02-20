
#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <pthread.h>

struct ionic_queue {
	void *va;
	uint32_t bytes; /* for munmap */
	uint32_t depth; /* no. of entries */
	uint32_t head;
	uint32_t tail;
	uint32_t stride;
	pthread_spinlock_t qlock;
};

static inline unsigned long get_aligned(uint32_t size, uint32_t al_size)
{
	return (unsigned long)(size + al_size - 1) & ~(al_size - 1);
}

static inline unsigned long roundup_pow_of_two(unsigned long val)
{
	unsigned long roundup = 1;

	if (val == 1)
		return (roundup << 1);

	while (roundup < val)
		roundup <<= 1;

	return roundup;
}

int ionic_alloc_aligned(struct ionic_queue *que, uint32_t pg_size);
void ionic_free_aligned(struct ionic_queue *que);

static inline void iowrite64(__u64 *dst, __u64 *src)
{
	*(volatile __u64 *)dst = *src;
}

static inline void iowrite32(__u32 *dst, __u32 *src)
{
	*(volatile __u32 *)dst = *src;
}

/* Basic queue operation */
static inline uint32_t ionic_is_que_full(struct ionic_queue *que)
{
	return (((que->tail+1) & (que->depth - 1)) == que->head);
}

static inline uint32_t ionic_is_que_empty(struct ionic_queue *que)
{
	return que->tail == que->head;
}

static inline uint32_t ionic_incr(uint32_t val, uint32_t max)
{
	return (++val & (max - 1));
}

static inline void ionic_incr_tail(struct ionic_queue *que)
{
	que->tail = ionic_incr(que->tail, que->depth);
}

static inline void ionic_incr_head(struct ionic_queue *que)
{
	que->head = ionic_incr(que->head, que->depth);
}

#endif
