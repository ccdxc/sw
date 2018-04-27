#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>

#include "../../../sim/pnso_sim.h"
#include "../../pnso_sim_osal.h"

#ifndef PNSO_ASSERT
extern void abort();
#define PNSO_ASSERT(x)  if (!(x)) { abort(); }
#endif

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
#define PNSO_SQ_DEPTH 16
struct pnso_sim_q {
	pthread_t thread;
	uint16_t head;
	uint16_t tail;
	volatile atomic_ulong num_processed;
	volatile atomic_ushort num_entries;
	volatile atomic_ushort num_batches;
	volatile bool stop_worker;
	pnso_sim_req_id_t ids[PNSO_SQ_DEPTH];
};

static const uint32_t g_req_entries_count = PNSO_MAX_SESSIONS * PNSO_SQ_DEPTH * 2;	/* TODO */
static struct pnso_sim_q_request g_req_entries[PNSO_MAX_SESSIONS *
					       PNSO_SQ_DEPTH * 2];

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

static struct pnso_sim_q g_req_queues[PNSO_MAX_SESSIONS];

static inline bool is_q_stopped(uint32_t sess_id)
{
	return g_req_queues[sess_id].stop_worker;
}

void pnso_sim_init_queues()
{
	struct pnso_sim_q *q;
	size_t i;

	/* initialize submission q */
	for (i = 0; i < PNSO_MAX_SESSIONS; i++) {
		q = &g_req_queues[i];
		memset(q, 0, sizeof(*q));
		atomic_init(&q->num_processed, 0);
		atomic_init(&q->num_entries, 0);
		atomic_init(&q->num_batches, 0);
	}

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

//void pnso_sim_sq_enqueue(uint32_t sess_id,

pnso_error_t pnso_sim_sq_enqueue(uint32_t sess_id,
				 enum pnso_batch_request batch_req,
				 struct pnso_service_request *svc_req,
				 struct pnso_service_result *svc_res,
				 completion_t cb,
				 void *cb_ctx, void **poll_ctx)
{
	struct pnso_sim_q *q = &g_req_queues[sess_id];
	pnso_sim_req_id_t id;
	struct pnso_sim_q_request *entry;
	uint16_t temp;

	/* Assumes check for not-full was already done */

	temp = atomic_fetch_add(&q->num_entries, 1);
	if (temp > PNSO_SQ_DEPTH) {
		/* TODO: error */
		atomic_fetch_sub(&q->num_entries, 1);
		return -1;
	}

	id = req_free_q_dequeue();
	q->ids[q->head++] = id;
	if (q->head == PNSO_SQ_DEPTH) {
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

	return 0;
}

struct pnso_sim_q_request *pnso_sim_sq_dequeue(uint32_t sess_id)
{
	struct pnso_sim_q *q = &g_req_queues[sess_id];
	pnso_sim_req_id_t id;
	struct pnso_sim_q_request *entry;

	/* Assumes check for not-empty was already done */

	id = q->ids[q->tail++];
	entry = req_id_to_entry(id);
	if (q->tail == PNSO_SQ_DEPTH) {
		q->tail = 0;
	}


	if (entry->end_of_batch) {
		atomic_fetch_sub(&q->num_batches, 1);
	}
	atomic_fetch_sub(&q->num_entries, 1);

	return entry;
}

bool pnso_sim_sq_is_empty(uint32_t sess_id)
{
	struct pnso_sim_q *q = &g_req_queues[sess_id];

	return atomic_load(&q->num_entries) == 0;
}

bool pnso_sim_sq_is_full(uint32_t sess_id)
{
	struct pnso_sim_q *q = &g_req_queues[sess_id];

	return atomic_load(&q->num_entries) >= PNSO_SQ_DEPTH;
}

bool pnso_sim_sq_is_batch_done(uint32_t sess_id)
{
	struct pnso_sim_q *q = &g_req_queues[sess_id];

	return atomic_load(&q->num_batches) > 0;
}

void pnso_sim_sq_wait_for_not_full(uint32_t sess_id)
{
	/* TODO: spinlock or condition variable */

	while (pnso_sim_sq_is_full(sess_id)) {
		usleep(1);
		if (is_q_stopped(sess_id)) {
			break;
		}
	}
}

void pnso_sim_sq_wait_for_not_empty(uint32_t sess_id)
{
	/* TODO: spinlock or condition variable */

	while (pnso_sim_sq_is_empty(sess_id)) {
		usleep(1);
		if (is_q_stopped(sess_id)) {
			break;
		}
	}
}

void pnso_sim_sq_wait_for_batch_done(uint32_t sess_id)
{
	/* TODO: spinlock or condition variable */

	while (!pnso_sim_sq_is_batch_done(sess_id)) {
		usleep(1);
		if (is_q_stopped(sess_id)) {
			break;
		}
	}
}

/*
 * Poll for completion of a particular request.
 * Returns true if request has been processed.
 */
bool pnso_sim_poll(uint32_t sess_id, void *poll_ctx)
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
		return true;
	}
#if 0
	size_t rc = atomic_load(&q->num_processed);

	if (rc) {
		atomic_fetch_sub(&q->num_processed, rc);
	}
#endif

	return false;
}

void pnso_sim_run_worker_once(uint32_t sess_id)
{
	struct pnso_sim_q *q = &g_req_queues[sess_id];
	struct pnso_sim_q_request *req;
	bool end_of_batch = false;

	if (!pnso_sim_sq_is_batch_done(sess_id)) {
		return;
	}

	while (!end_of_batch) {
		/* Get next request */
		req = pnso_sim_sq_dequeue(sess_id);
		end_of_batch = req->end_of_batch;

		/* Execute request */
		pnso_sim_execute_request(sess_id,
					 req->svc_req, req->svc_res,
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
	uint32_t sess_id = (uint32_t) (uint64_t) opaque;

	while (1) {
		if (is_q_stopped(sess_id)) {
			break;
		}

		pnso_sim_sq_wait_for_batch_done(sess_id);
		pnso_sim_run_worker_once(sess_id);
	}

	return NULL;
}

void pnso_sim_start_worker_thread(uint32_t sess_id)
{
	pthread_t *t = &g_req_queues[sess_id].thread;

	g_req_queues[sess_id].stop_worker = false;

	if (0 !=
	    pthread_create(t, NULL, pnso_sim_run_worker_loop,
			   (void *) (uint64_t) sess_id)) {
		PNSO_ASSERT(0);
	}
}

void pnso_sim_stop_worker_thread(uint32_t sess_id)
{
	void *ret;
	struct pnso_sim_q *q = &g_req_queues[sess_id];
	q->stop_worker = true;

	pthread_join(g_req_queues[sess_id].thread, &ret);
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
