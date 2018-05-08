#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "pnso_sim.h"
#include "pnso_sim_osal.h"

/* using exactly 64k simplifies wrapping case */
struct {
	volatile atomic_ushort head;
	volatile atomic_ushort tail;
	volatile pnso_sim_req_id_t req_ids[64 * 1024];
} g_req_free_q;

static void req_free_q_enqueue(pnso_sim_req_id_t id)
{
	/* Assumes the queue is not full (always valid for queue larger than entry count) */

	g_req_free_q.req_ids[atomic_fetch_add(&g_req_free_q.head, 1)] = id;
}

static pnso_sim_req_id_t req_free_q_dequeue()
{
	pnso_sim_req_id_t id;

	/* Assumes the queue is not empty (TODO) */

	id = g_req_free_q.req_ids[atomic_fetch_add(&g_req_free_q.tail, 1)];

	return id;
}



/* Assumptions:
 * - all enqueue operations for a given session happen on a single thread
 * - all dequeue operations for a given session happen on a single thread
 */
struct pnso_sim_q {
	pthread_t thread;
	uint16_t depth;
	uint16_t head;
	uint16_t tail;
	volatile atomic_ulong num_processed;
	volatile atomic_ushort num_entries;
	volatile atomic_ushort num_batches;
	volatile bool stop_worker;
	pnso_sim_req_id_t ids[0];
};

static const uint32_t g_req_entries_count = PNSO_SIM_MAX_SESSIONS * PNSO_SIM_DEFAULT_SQ_DEPTH * 2;	/* TODO */

static struct pnso_sim_q_request g_req_entries[PNSO_SIM_MAX_SESSIONS * PNSO_SIM_DEFAULT_SQ_DEPTH * 2];

static inline struct pnso_sim_q_request *req_id_to_entry(pnso_sim_req_id_t
							 id)
{
	PNSO_ASSERT(id);
	return &g_req_entries[id - 1];
}

static inline pnso_sim_req_id_t req_entry_to_id(struct pnso_sim_q_request
						*entry)
{
	return (pnso_sim_req_id_t) ((((uint64_t) entry -
				      (uint64_t) g_req_entries) /
				     sizeof(struct pnso_sim_q_request)) +
				    1);
}

/* Thread local storage */
static _Thread_local struct pnso_sim_worker_ctx t_worker_ctx = { NULL, NULL };

struct pnso_sim_worker_ctx *pnso_sim_get_worker_ctx()
{
	return &t_worker_ctx;
}

static inline bool is_q_stopped(struct pnso_sim_q *q)
{
	return q->stop_worker;
}

bool pnso_sim_is_worker_running()
{
	if (!t_worker_ctx.req_q) {
		return false;
	}

	return !t_worker_ctx.req_q->stop_worker;
}

/* OSAL wrapper for malloc/free */
void *pnso_sim_alloc(size_t sz)
{
	return malloc(sz);
}

void pnso_sim_free(void *ptr)
{
	free(ptr);
}

/* Global initialization */
void pnso_sim_init_req_pool()
{
	size_t i;

	/* initialize free q */
	for (i = 0; i < 64 * 1024; i++) {
		if (i < g_req_entries_count - 1) {
			g_req_free_q.req_ids[i] = i + 1;
		} else {
			g_req_free_q.req_ids[i] = 0;
		}
	}
	atomic_init(&g_req_free_q.head, g_req_entries_count - 1);
	atomic_init(&g_req_free_q.tail, 0);
}

/* Per-thread initialization, called outside worker. */
pnso_error_t pnso_sim_init_req_queue(uint16_t depth)
{
	struct pnso_sim_q *q;
	uint32_t qsize = sizeof(struct pnso_sim_q) + ((uint32_t) depth * sizeof(pnso_sim_req_id_t));

	if (depth < 1) {
		return EINVAL;
	}

	if (t_worker_ctx.req_q != NULL) {
		return EEXIST;
	}

	/* allocate queue */
	q = pnso_sim_alloc(qsize);
	if (!q) {
		return ENOMEM;
	}

	/* initialize */
	memset(q, 0, sizeof(*q));
	q->depth = depth;
	atomic_init(&q->num_processed, 0);
	atomic_init(&q->num_entries, 0);
	atomic_init(&q->num_batches, 0);

	t_worker_ctx.req_q = q;

	return PNSO_OK;
}

/* Per-thread cleanup, called outside worker. */
void pnso_sim_finit_req_queue()
{
	struct pnso_sim_q *q = t_worker_ctx.req_q;

	/* deallocate */
	if (q) {
		t_worker_ctx.req_q = NULL;
		pnso_sim_free(q);
	}
}

pnso_error_t pnso_sim_sq_enqueue(enum pnso_batch_request batch_req,
				 struct pnso_service_request *svc_req,
				 struct pnso_service_result *svc_res,
				 completion_t cb,
				 void *cb_ctx, void **poll_ctx)
{
	struct pnso_sim_q *q = pnso_sim_get_worker_ctx()->req_q;
	pnso_sim_req_id_t id;
	struct pnso_sim_q_request *entry;
	uint16_t temp;

	PNSO_ASSERT(q);

	/* Assumes check for not-full was already done */

	temp = atomic_fetch_add(&q->num_entries, 1);
	if (temp >= q->depth) {
		/* TODO: error */
		atomic_fetch_sub(&q->num_entries, 1);
		return EAGAIN;
	}

	id = req_free_q_dequeue();
	q->ids[q->head++] = id;
	if (q->head >= q->depth) {
		q->head = 0;
	}

	entry = req_id_to_entry(id);
	entry->svc_req = svc_req;
	entry->svc_res = svc_res;
	entry->cb = cb;
	entry->cb_ctx = cb_ctx;
	entry->end_of_batch = (batch_req == PNSO_BATCH_REQ_FLUSH);
	entry->is_proc_done = false;
	entry->id = id;

	if (poll_ctx) {
		*poll_ctx = (void *) (uint64_t) id;
		entry->poll_mode = true;
	} else {
		entry->poll_mode = false;
	}

	if (entry->end_of_batch) {
		atomic_fetch_add(&q->num_batches, 1);
	}

	return PNSO_OK;
}

struct pnso_sim_q_request *pnso_sim_q_dequeue(struct pnso_sim_q *q)
{
	pnso_sim_req_id_t id;
	struct pnso_sim_q_request *entry;

	/* Assumes check for not-empty was already done */

	id = q->ids[q->tail++];
	entry = req_id_to_entry(id);
	if (q->tail >= q->depth) {
		q->tail = 0;
	}


	if (entry->end_of_batch) {
		atomic_fetch_sub(&q->num_batches, 1);
	}
	atomic_fetch_sub(&q->num_entries, 1);

	return entry;
}

bool pnso_sim_q_is_empty(struct pnso_sim_q *q)
{
	return atomic_load(&q->num_entries) == 0;
}

bool pnso_sim_q_is_full(struct pnso_sim_q *q)
{
	return atomic_load(&q->num_entries) >= q->depth;
}

bool pnso_sim_q_is_batch_done(struct pnso_sim_q *q)
{
	return atomic_load(&q->num_batches) > 0;
}

void pnso_sim_q_wait_for_not_full(struct pnso_sim_q *q)
{
	/* TODO: spinlock or condition variable */

	while (pnso_sim_q_is_full(q)) {
		usleep(1);
		if (is_q_stopped(q)) {
			break;
		}
	}
}

void pnso_sim_q_wait_for_not_empty(struct pnso_sim_q *q)
{
	/* TODO: spinlock or condition variable */

	while (pnso_sim_q_is_empty(q)) {
		usleep(1);
		if (is_q_stopped(q)) {
			break;
		}
	}
}

void pnso_sim_q_wait_for_batch_done(struct pnso_sim_q *q)
{
	/* TODO: spinlock or condition variable */

	while (!pnso_sim_q_is_batch_done(q)) {
		usleep(1);
		if (is_q_stopped(q)) {
			break;
		}
	}
}

/*
 * Poll for completion of a particular request.
 * Returns PNSO_OK if request has been processed, else EAGAIN.
 */
pnso_error_t pnso_sim_poll(void *poll_ctx)
{
	struct pnso_sim_q_request *entry;

	pnso_sim_req_id_t id = (pnso_sim_req_id_t) (uint64_t) poll_ctx;
	entry = req_id_to_entry(id);
	if (entry->is_proc_done) {
		if (entry->cb) {
			entry->cb(entry->cb_ctx, entry->svc_res);
		}
		/* Restore entry to free list */
		req_free_q_enqueue(id);
		return PNSO_OK;
	}
#if 0
	size_t rc = atomic_load(&q->num_processed);

	if (rc) {
		atomic_fetch_sub(&q->num_processed, rc);
	}
#endif

	return EAGAIN;
}

/*
 * Poll for completion of a particular request.
 * Does not return until request has been processed.
 */
pnso_error_t pnso_sim_poll_wait(void *poll_ctx)
{
	pnso_error_t rc;

	while (EAGAIN == (rc = pnso_sim_poll(poll_ctx))) {
		if (!pnso_sim_is_worker_running()) {
			return ENODEV; /* TODO */
		}
		usleep(1);
	}
	return rc;
}

void pnso_sim_run_worker_once(struct pnso_sim_worker_ctx *worker_ctx)
{
	struct pnso_sim_q *q = worker_ctx->req_q;
	struct pnso_sim_q_request *req;
	bool end_of_batch = false;

	if (!pnso_sim_q_is_batch_done(q)) {
		return;
	}

	while (!end_of_batch) {
		/* Get next request */
		req = pnso_sim_q_dequeue(q);
		end_of_batch = req->end_of_batch;

		/* Execute request */
		pnso_sim_execute_request(worker_ctx, req->svc_req, req->svc_res,
					 req->cb, req->cb_ctx);

		atomic_fetch_add(&q->num_processed, 1);

		/* Call callback directly, if polling is not enabled */
		if (req->cb && !req->poll_mode) {
			req->cb(req->cb_ctx, req->svc_res);
			/* Restore entry to free list */
			req_free_q_enqueue(req->id);
		} else {
			req->is_proc_done = true;
		}

	}
	while (!req->end_of_batch);
}

void *pnso_sim_run_worker_loop(void *opaque)
{
	struct pnso_sim_worker_ctx *worker_ctx = (struct pnso_sim_worker_ctx *) opaque;
	struct pnso_sim_q *q = worker_ctx->req_q;

	while (1) {
		if (is_q_stopped(q)) {
			break;
		}

		pnso_sim_q_wait_for_batch_done(q);
		pnso_sim_run_worker_once(worker_ctx);
	}

	return NULL;
}

pnso_error_t pnso_sim_start_worker_thread()
{
	pnso_error_t rc;

	rc = pnso_sim_init_req_queue(g_init_params.per_core_qdepth);
	if (rc != PNSO_OK) {
		return rc;
	}

	t_worker_ctx.req_q->stop_worker = false;

	rc = pthread_create(&t_worker_ctx.req_q->thread, NULL, pnso_sim_run_worker_loop,
			    (void *) &t_worker_ctx);
	return rc;
}

void pnso_sim_stop_worker_thread()
{
	void *ret;
	struct pnso_sim_q *q = t_worker_ctx.req_q;

	PNSO_ASSERT(q);

	q->stop_worker = true;

	pthread_join(q->thread, &ret);
	pnso_sim_finit_req_queue();
}

struct slab_desc {
	uint8_t *data;
	uint32_t total;
	volatile atomic_ulong allocated;
};

struct slab_desc *pnso_sim_slab_init(uint8_t * data, uint32_t size)
{
	if (size < sizeof(struct slab_desc)) {
		return NULL;
	}
	struct slab_desc *slab = (struct slab_desc *) data;
	slab->data = data;
	slab->total = size;
	slab->allocated = sizeof(struct slab_desc);

	return slab;
}

void *pnso_sim_slab_alloc(struct slab_desc *slab, uint32_t size)
{
	void *ret = NULL;
	uint32_t temp;

	temp = atomic_fetch_add(&slab->allocated, size);
	if (slab->total >= (temp + size)) {
		ret = (void *) slab->data + temp;
	} else {
		atomic_fetch_sub(&slab->allocated, temp);
	}
	return ret;
}
