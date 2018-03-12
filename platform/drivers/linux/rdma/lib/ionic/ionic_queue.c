#include <errno.h>

#include "ionic_queue.h"
#include "memory.h"

int ionic_queue_init(struct ionic_queue *q, size_t pg_size,
		     uint16_t depth, uint16_t stride)
{
	depth = ionic_u16_mask(depth);
	stride = ionic_u16_power(stride);

	if (!depth || !stride)
		return EINVAL;

	q->size = ((uint32_t)depth + 1) * stride;

	if (q->size < pg_size)
		q->size = pg_size;

	q->ptr = ionic_map_anon(q->size);
	if (!q->ptr)
		return errno;

	q->prod = 0;
	q->cons = 0;
	q->mask = depth;
	q->stride = stride;
	q->dbell = 0;

	return 0;
}

void ionic_queue_destroy(struct ionic_queue *q)
{
	ionic_unmap(q->ptr, q->size);
}
