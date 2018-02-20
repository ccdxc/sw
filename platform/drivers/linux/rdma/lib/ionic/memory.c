
#include <string.h>
#include <sys/mman.h>

#include "main.h"

int ionic_alloc_aligned(struct ionic_queue *que, uint32_t pg_size)
{
	int ret, bytes;

	bytes = (que->depth * que->stride);
	que->bytes = get_aligned(bytes, pg_size);
	que->va = mmap(NULL, que->bytes, PROT_READ | PROT_WRITE,
		       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (que->va == MAP_FAILED) {
		que->bytes = 0;
		return errno;
	}
	/* Touch pages before proceeding. */
	memset(que->va, 0, que->bytes);

	ret = ibv_dontfork_range(que->va, que->bytes);
	if (ret) {
		munmap(que->va, que->bytes);
		que->bytes = 0;
	}

	return ret;
}

void ionic_free_aligned(struct ionic_queue *que)
{
	if (que->bytes) {
		ibv_dofork_range(que->va, que->bytes);
		munmap(que->va, que->bytes);
		que->bytes = 0;
	}
}
