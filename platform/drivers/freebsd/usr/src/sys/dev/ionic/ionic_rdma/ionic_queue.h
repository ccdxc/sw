#ifndef IONIC_QUEUE_H
#define IONIC_QUEUE_H

#include <linux/io.h>

#define IONIC_QID_MASK		((1ull << 24) - 1)
#define IONIC_DBELL_QID_SHIFT	24
#define IONIC_DBELL_RING_ARM	(1ull << 16)
#define IONIC_DBELL_RING_ARM_SOLICITED	(1ull << 17)

/** ionic_u16_mask - Round u16 up to power of two minus one.
 * @val:	Value to round up.
 *
 * This uses cumulative shift to get the next greater or equal power of two
 * minus one.  If val is already a power of two minus one, the result will be
 * the same value.
 *
 * Return: power of two minus one.
 */
static inline u16 ionic_u16_mask(u16 val)
{
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;

	return val;
}

/** ionic_u16_power - Round u16 up to power of two.
 * @val:	Value to round up.
 *
 * Get the next greater or equal power of two.  If val is already a power of
 * two, the result will be the same value.
 *
 * The value may round "up" to zero, which means (2^16).
 *
 * Return: power of two.
 */
static inline u16 ionic_u16_power(u16 val)
{
	return ionic_u16_mask(val - 1u) + 1u;
}

/** struct ionic_queue - Ring buffer used between device and driver.
 * @size:	Size of the buffer, in bytes.
 * @dma:	Dma address of the buffer.
 * @ptr:	Buffer virtual address.
 * @prod:	Driver position in the queue.
 * @cons:	Device position in the queue, for to-device queues.
 *              Current color of the queue, for from-device queues.
 * @mask:	Capacity of the queue, subtracting the hole.
 *		This value is a power of two minus one.
 * @stride:	Size of an element in the queue.
 *		Some queues require alignment, or a power of two.
 * @dbell:	Doorbell identifying bits.
 */
struct ionic_queue {
	size_t size;
	dma_addr_t dma;
	void *ptr;
	u16 prod;
	u16 cons;
	u16 mask;
	u16 stride;
	u64 dbell;
};

/** ionic_queue_init - Initialize user space queue.
 * @q:		Uninitialized queue structure.
 * @dma_dev:	DMA device for mapping.
 * @depth:	Power-of-two depth of the queue.  Zero means 2^16.
 * @stride:	Size of each element of the queue.
 *
 * Return: status code.
 */
int ionic_queue_init(struct ionic_queue *q, struct device *dma_dev,
		     u16 depth, u16 stride);

/** ionic_queue_destroy - Destroy user space queue.
 * @q:		Queue structure.
 * @dma_dev:	DMA device for mapping.
 *
 * Return: status code.
 */
void ionic_queue_destroy(struct ionic_queue *q, struct device *dma_dev);

/** ionic_queue_empty - Test if queue is empty.
 * @q:		Queue structure.
 *
 * This is only valid for to-device queues.
 *
 * Return: is empty.
 */
static inline bool ionic_queue_empty(struct ionic_queue *q)
{
	return q->prod == q->cons;
}

/** ionic_queue_length - Get the current length of the queue.
 * @q:		Queue structure.
 *
 * This is only valid for to-device queues.
 *
 * Return: length.
 */
static inline u16 ionic_queue_length(struct ionic_queue *q)
{
	return (q->prod - q->cons) & q->mask;
}

/** ionic_queue_length_remaining - Get the remaining length of the queue.
 * @q:		Queue structure.
 *
 * This is only valid for to-device queues.
 *
 * Return: length remaining.
 */
static inline u16 ionic_queue_length_remaining(struct ionic_queue *q)
{
	return q->mask - ionic_queue_length(q);
}

/** ionic_queue_full - Test if queue is full.
 * @q:		Queue structure.
 *
 * This is only valid for to-device queues.
 *
 * Return: is full.
 */
static inline bool ionic_queue_full(struct ionic_queue *q)
{
	return q->mask == ionic_queue_length(q);
}

/** ionic_queue_color_init - Initialize the color to one.
 * @q:		Queue structure.
 *
 * This should be called after ionic_queue_init() for from-device queue.
 */
static inline void ionic_queue_color_init(struct ionic_queue *q)
{
	/* the cons field is used for color */
	q->cons = true;
}

/** ionic_queue_color - Get the current color or phase of the queue.
 * @q:		Queue structure.
 *
 * This is only valid for from-device queues (events, completions).
 *
 * Return: color.
 */
static inline bool ionic_queue_color(struct ionic_queue *q)
{
	/* the cons field is used for color */
	return q->cons;
}

/** ionic_queue_color_wrap - Swap the queue color if wrapped.
 * @q:		Queue structure.
 *
 * This should be called after ionic_queue_produce() for from-device queue.
 */
static inline void ionic_queue_color_wrap(struct ionic_queue *q)
{
	/* cons logical xor with (prod == 0) */
	q->cons = q->cons != (q->prod == 0);
}

/** ionic_queue_at - Get the element at the given index.
 * @q:		Queue structure.
 * @idx:	Index in the queue.
 *
 * The index must be within the bounds of the queue.  It is not checked here.
 *
 * Return: pointer to element at index.
 */
static inline void *ionic_queue_at(struct ionic_queue *q, u16 idx)
{
	return q->ptr + idx * q->stride;
}

/** ionic_queue_at_prod - Get the element at the producer index.
 * @q:		Queue structure.
 *
 * Return: pointer to element at producer index.
 */
static inline void *ionic_queue_at_prod(struct ionic_queue *q)
{
	return ionic_queue_at(q, q->prod);
}

/** ionic_queue_at_cons - Get the element at the consumer index.
 * @q:		Queue structure.
 *
 * Return: pointer to element at consumer index.
 */
static inline void *ionic_queue_at_cons(struct ionic_queue *q)
{
	return ionic_queue_at(q, q->cons);
}

/** ionic_queue_produce - Increase the producer index.
 * @q:		Queue structure.
 *
 * Caller must ensure that the queue is not full.  It is not checked here.
 */
static inline void ionic_queue_produce(struct ionic_queue *q)
{
	q->prod = (q->prod + 1) & q->mask;
}

/** ionic_queue_consume - Increase the consumer index.
 * @q:		Queue structure.
 *
 * Caller must ensure that the queue is not empty.  It is not checked here.
 *
 * This is only valid for to-device queues.
 */
static inline void ionic_queue_consume(struct ionic_queue *q)
{
	q->cons = (q->cons + 1) & q->mask;
}

/** ionic_queue_dbell_init - Initialize doorbell bits for queue id.
 * @q:		Queue structure.
 * @qid:	Queue identifying number.
 */
static inline void ionic_queue_dbell_init(struct ionic_queue *q,
					  u32 qid)
{
	q->dbell = ((u64)qid & IONIC_QID_MASK) << IONIC_DBELL_QID_SHIFT;
}

/** ionic_queue_dbell_val - Get current doorbell update value.
 * @q:		Queue structure.
 */
static inline u64 ionic_queue_dbell_val(struct ionic_queue *q)
{
	return q->dbell | q->prod;
}

/** ionic_dbell_ring - Write the doorbell value to register.
 * @dbreg:	Doorbell register.
 * @val:	Doorbell value from queue.
 */
static inline void ionic_dbell_ring(u64 __iomem *dbreg, u64 val)
{
	/* XXX should be iowrite64, but waiting on Logan's patch lib/iomap.c */
	writeq(val, dbreg);
}

#endif /* IONIC_QUEUE_H */
