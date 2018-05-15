#include <linux/dma-mapping.h>

#include "ionic_queue.h"

int ionic_queue_init(struct ionic_queue *q, struct device *dma_dev,
		     uint16_t depth, uint16_t stride)
{
	depth = ionic_u16_mask(depth);
	stride = ionic_u16_power(stride);

	if (!depth || !stride)
		return -EINVAL;

	q->size = ((uint32_t)depth + 1) * stride;
	if (q->size < PAGE_SIZE) {
		q->size = PAGE_SIZE;
		depth = PAGE_SIZE / stride - 1;
	}

	q->ptr = dma_zalloc_coherent(dma_dev, q->size, &q->dma, GFP_KERNEL);
	if (!q->ptr)
		return -ENOMEM;

	/* it will always be page aligned, but just to be sure... */
	if (WARN_ON(!PAGE_ALIGNED(q->ptr))) {
		dma_free_coherent(dma_dev, q->size, q->ptr, q->dma);
		return -ENOMEM;
	}

	q->prod = 0;
	q->cons = 0;
	q->mask = depth;
	q->stride = stride;
	q->dbell = 0;

	return 0;
}

void ionic_queue_destroy(struct ionic_queue *q, struct device *dma_dev)
{
	dma_free_coherent(dma_dev, q->size, q->ptr, q->dma);
}
