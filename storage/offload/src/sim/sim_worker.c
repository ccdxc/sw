/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */

#include "osal_errno.h"
#include "osal_mem.h"
#include "osal_sys.h"
#include "osal_thread.h"
#include "osal_atomic.h"

#include "sim.h"
#include "sim_chain.h"
#include "sim_worker.h"

#define SIM_MAX_CPU_CORES 128

/* Must be a power of two */
#define SIM_FREE_LIST_COUNT (64 * 1024)
#define SIM_FREE_LIST_MASK (SIM_FREE_LIST_COUNT - 1)

struct {
	osal_atomic_int_t head;
	osal_atomic_int_t tail;
	volatile sim_req_id_t req_ids[SIM_FREE_LIST_COUNT];
} g_req_free_q;

/*
 * Assumption: single producer, single consumer
 */
struct sim_q {
	uint16_t depth;
	uint16_t head;
	uint16_t tail;
	osal_atomic_int_t num_processed;
	osal_atomic_int_t num_entries;
	osal_atomic_int_t num_batches;
	sim_req_id_t ids[0];
};

static uint32_t g_req_entries_count;
static struct sim_q_request *g_req_entries;

static osal_atomic_int_t g_worker_count;
static osal_atomic_int_t g_worker_lock;
static osal_thread_t g_worker_threads[SIM_MAX_SESSIONS];
static struct sim_worker_ctx g_worker_ctxs[SIM_MAX_SESSIONS];

static struct sim_worker_ctx *g_per_core_worker_ctx[SIM_MAX_CPU_CORES];
static osal_atomic_int_t g_per_core_sync_done[SIM_MAX_CPU_CORES];

static inline struct sim_q_request *req_id_to_entry(sim_req_id_t id)
{
	PNSO_ASSERT(id);
	return &g_req_entries[id - 1];
}

#if 0
static inline sim_req_id_t req_entry_to_id(struct sim_q_request	*entry)
{
	return (sim_req_id_t) ((((uint64_t) entry -
				 (uint64_t) g_req_entries) /
				sizeof(struct sim_q_request)) +
			       1);
}
#endif

static void req_free_q_enqueue(sim_req_id_t id)
{
	/* Assumes the queue is not full (choose queue size > entry count) */
	int i = osal_atomic_fetch_add(&g_req_free_q.head, 1);

	g_req_free_q.req_ids[(uint32_t) i & SIM_FREE_LIST_MASK] = id;
}

static sim_req_id_t req_free_q_dequeue(void)
{
	sim_req_id_t id;
	int i = osal_atomic_fetch_add(&g_req_free_q.tail, 1);

	id = g_req_free_q.req_ids[(uint32_t) i & SIM_FREE_LIST_MASK];
	if (!id) {
		/* No more entries, fix the tail */
		osal_atomic_fetch_sub(&g_req_free_q.tail, 1);
	}

	return id;
}

static void req_free_q_batch_enqueue(struct sim_q_request *req)
{
	sim_req_id_t id;

	while (req) {
		id = req->next_id;

		/* Restore entry to free list */
		req_free_q_enqueue(req->id);
		
		req = id ? req_id_to_entry(id) : NULL;
	}
}

void sim_workers_spinlock(void)
{
	osal_atomic_lock(&g_worker_lock);
}

void sim_workers_spinunlock(void)
{
	osal_atomic_unlock(&g_worker_lock);
}

static struct sim_worker_ctx *sim_alloc_worker_ctx(void)
{
	uint32_t worker_id = osal_atomic_fetch_add(&g_worker_count, 1);

	if (worker_id >= SIM_MAX_SESSIONS) {
		osal_atomic_fetch_sub(&g_worker_count, 1);
		return NULL;
	}

	return &g_worker_ctxs[worker_id];
}

static struct sim_worker_ctx *sim_lookup_worker_ctx(int core_id)
{
	return g_per_core_worker_ctx[core_id];
}

struct sim_worker_ctx *sim_get_worker_ctx(int core_id)
{
	struct sim_worker_ctx *wctx;

	if (core_id < 0 || core_id >= SIM_MAX_CPU_CORES) {
		return NULL;
	}

	wctx = g_per_core_worker_ctx[core_id];
	if (!wctx) {
		sim_workers_spinlock();
		wctx = g_per_core_worker_ctx[core_id];
		if (!wctx) {
			wctx = sim_alloc_worker_ctx();
			if (wctx) {
				wctx->core_id = core_id;
				g_per_core_worker_ctx[core_id] = wctx;
			}
		}
		sim_workers_spinunlock();
	}
	return wctx;
}

/*
 * For now there is no graceful thread shutdown,
 *   so always returns false.  TODO.
 */
static inline bool is_worker_stopping(int core_id)
{
	struct sim_worker_ctx *wctx = sim_lookup_worker_ctx(core_id);

	if (!wctx) {
		return true;
	}
	return osal_thread_should_stop(wctx->worker);
}

bool sim_is_worker_running(int core_id)
{
	struct sim_worker_ctx *wctx = sim_lookup_worker_ctx(core_id);

	if (!wctx) {
		return false;
	}
	return osal_thread_is_running(wctx->worker);
}

/* Global initialization */
pnso_error_t sim_init_req_pool(uint32_t max_reqs)
{
	size_t i;

	/* Cannot exceed static size of free list */
	if (max_reqs >= SIM_FREE_LIST_COUNT) {
		max_reqs = SIM_FREE_LIST_COUNT - 1;
	}

	/* Allocate request entries themselves */
	g_req_entries = osal_alloc(sizeof(struct sim_q_request) * max_reqs);
	if (g_req_entries) {
		memset(g_req_entries, 0,
		       sizeof(struct sim_q_request) * max_reqs);
		g_req_entries_count = max_reqs;
	} else {
		g_req_entries_count = 0;
	}

	/* initialize free q */
	for (i = 0; i < SIM_FREE_LIST_COUNT; i++) {
		if (i < g_req_entries_count) {
			g_req_free_q.req_ids[i] = i + 1;
		} else {
			g_req_free_q.req_ids[i] = 0;
		}
	}
	osal_atomic_init(&g_req_free_q.head, g_req_entries_count - 1);
	osal_atomic_init(&g_req_free_q.tail, 0);

	for (i = 0; i < SIM_MAX_CPU_CORES; i++) {
		osal_atomic_init(&g_per_core_sync_done[i], 0);
	}

	if (g_req_entries_count == 0) {
		return ENOMEM;
	}
	return PNSO_OK;
}

static struct sim_q *sim_alloc_queue(uint16_t depth)
{
	struct sim_q *q;
	uint32_t qsize = sizeof(struct sim_q) +
		((uint32_t) depth * sizeof(sim_req_id_t));

	if (depth < 1) {
		return NULL;
	}

	/* allocate queue */
	q = osal_alloc(qsize);
	if (!q) {
		return NULL;
	}

	/* initialize */
	memset(q, 0, sizeof(*q));
	q->depth = depth;
	osal_atomic_init(&q->num_processed, 0);
	osal_atomic_init(&q->num_entries, 0);
	osal_atomic_init(&q->num_batches, 0);

	return q;
}

pnso_error_t sim_init_worker_pool(uint32_t max_q_depth)
{
	size_t i;

	if (max_q_depth > SIM_MAX_Q_DEPTH) {
		max_q_depth = SIM_MAX_Q_DEPTH;
	}

	for (i = 0; i < SIM_MAX_SESSIONS; i++) {
		g_worker_ctxs[i].req_q = sim_alloc_queue(max_q_depth);
		if (!g_worker_ctxs[i].req_q) {
			return ENOMEM;
		}
		memset(&g_worker_threads[i], 0, sizeof(g_worker_threads[i]));
		g_worker_ctxs[i].worker = &g_worker_threads[i];
		g_worker_ctxs[i].sess = NULL;
	}
	memset(g_per_core_worker_ctx, 0, sizeof(g_per_core_worker_ctx));

	osal_atomic_init(&g_worker_count, 0);
	osal_atomic_init(&g_worker_lock, 0);

	return PNSO_OK;
}

pnso_error_t sim_sq_enqueue(int core_id,
			    struct pnso_service_request *svc_req,
			    struct pnso_service_result *svc_res,
			    completion_cb_t cb,
			    void *cb_ctx, void **poll_ctx,
			    bool flush)
{
	struct sim_worker_ctx *wctx = sim_get_worker_ctx(core_id);
	struct sim_q *q = wctx ? wctx->req_q : NULL;
	sim_req_id_t id;
	struct sim_q_request *entry;
	uint16_t temp;

	if (!q) {
		return EINVAL;
	}

	/* Assumes check for not-full was already done */

	temp = osal_atomic_fetch_add(&q->num_entries, 1);
	if (temp >= q->depth) {
		/* TODO: error */
		osal_atomic_fetch_sub(&q->num_entries, 1);
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
	entry->end_of_batch = flush; /*(cb == NULL) ? false : true;*/
	entry->is_proc_done = false;
	entry->id = id;

	if (poll_ctx) {
		*poll_ctx = (void *) (uint64_t) id;
		entry->poll_mode = true;
	} else {
		entry->poll_mode = false;
	}

	if (entry->end_of_batch) {
		osal_atomic_fetch_add(&q->num_batches, 1);
	}

	return PNSO_OK;
}

pnso_error_t sim_sq_flush(int core_id,
			  completion_cb_t cb,
			  void *cb_ctx, void **poll_ctx)
{
	struct sim_worker_ctx *wctx = sim_get_worker_ctx(core_id);
	struct sim_q *q = wctx ? wctx->req_q : NULL;
	struct sim_q_request *entry;
	sim_req_id_t id;

	if (!q) {
		return EINVAL;
	}

	/* Verify there are entries to flush */
	if (osal_atomic_read(&q->num_entries) <= 0) {
		/* Attempted to flush 0 entries */
		return EINVAL;
	}

	/* Find id of final request */
	if (q->head == 0) {
		id = q->ids[q->depth - 1];
	} else {
		id = q->ids[q->head - 1];
	}
	entry = req_id_to_entry(id);
	if (entry->end_of_batch) {
		/* Already marked as batch end, flush is redundant */
		return EINVAL;
	}

	/* Mark entry as batch end */
	entry->cb = cb;
	entry->cb_ctx = cb_ctx;
	if (poll_ctx) {
		*poll_ctx = (void *) (uint64_t) id;
		entry->poll_mode = true;
	} else {
		entry->poll_mode = false;
	}
	entry->end_of_batch = true;
	osal_atomic_fetch_add(&q->num_batches, 1);

	return PNSO_OK;
}

static struct sim_q_request *sim_q_dequeue(struct sim_q *q)
{
	sim_req_id_t id;
	struct sim_q_request *entry;

	/* Assumes check for not-empty was already done */

	id = q->ids[q->tail++];
	entry = req_id_to_entry(id);
	if (q->tail >= q->depth) {
		q->tail = 0;
	}

	if (entry->end_of_batch) {
		osal_atomic_fetch_sub(&q->num_batches, 1);
	}

	osal_atomic_fetch_sub(&q->num_entries, 1);

	return entry;
}

#if 0
static bool sim_q_is_empty(struct sim_q *q)
{
	return osal_atomic_read(&q->num_entries) <= 0;
}

static bool sim_q_is_full(struct sim_q *q)
{
	return osal_atomic_read(&q->num_entries) >= q->depth;
}

static void sim_q_wait_for_not_full(struct sim_q *q, int core_id)
{
	while (sim_q_is_full(q)) {
		osal_yield();
		if (is_worker_stopping(core_id)) {
			break;
		}
	}
}

static void sim_q_wait_for_not_empty(struct sim_q *q, int core_id)
{
	while (sim_q_is_empty(q)) {
		osal_yield();
		if (is_worker_stopping(core_id)) {
			break;
		}
	}
}
#endif

static bool sim_q_is_batch_done(struct sim_q *q)
{
	return osal_atomic_read(&q->num_batches) > 0;
}

static void sim_q_wait_for_batch_done(struct sim_q *q, int core_id)
{
	while (!sim_q_is_batch_done(q)) {
		osal_yield();
		if (is_worker_stopping(core_id)) {
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
	struct sim_q_request *entry;
	sim_req_id_t id = (sim_req_id_t) (uint64_t) poll_ctx;

	if (!id) {
		return EINVAL;
	}

	entry = req_id_to_entry(id);
	if (!entry->poll_mode) {
		return EINVAL;
	}
	if (entry->is_proc_done) {
		if (entry->cb) {
			entry->cb(entry->cb_ctx, entry->svc_res);
		}
		/* Restore entries to free list */
		req_free_q_batch_enqueue(entry);
		return PNSO_OK;
	}

	return EAGAIN;
}

/*
 * Poll for completion of a particular request.
 * Does not return until request has been processed.
 */
pnso_error_t pnso_sim_poll_wait(void *poll_ctx, int core_id)
{
	pnso_error_t rc;

	while (EAGAIN == (rc = pnso_sim_poll(poll_ctx))) {
		if (!sim_is_worker_running(core_id)) {
			return ENODEV; /* TODO */
		}
		osal_yield();
	}
	return rc;
}

/*
 * Polling loop for sync callback completion on a particular core.
 */
pnso_error_t pnso_sim_sync_wait(int core_id)
{
	while (0 == osal_atomic_read(&g_per_core_sync_done[core_id])) {
		osal_yield();
		if (is_worker_stopping(core_id)) {
			break;
		}
	}

	/* clear it for next time */
	osal_atomic_set(&g_per_core_sync_done[core_id], 0);
	return PNSO_OK;
}

void pnso_sim_sync_completion_cb(void *cb_ctx, struct pnso_service_result *svc_res)
{
	int core_id = (int) (uint64_t) cb_ctx;

	if (core_id < 0 || core_id >= SIM_MAX_CPU_CORES) {
		return;
	}

	osal_atomic_fetch_add(&g_per_core_sync_done[core_id], 1);
}

void pnso_sim_run_worker_once(struct sim_worker_ctx *wctx)
{
	struct sim_q *q = wctx->req_q;
	struct sim_q_request *req = NULL;
	sim_req_id_t next_id = 0;
	bool end_of_batch = false;

	if (!sim_q_is_batch_done(q)) {
		return;
	}

	/* Execute requests in batch */
	while (!end_of_batch) {
		/* Get next request */
		req = sim_q_dequeue(q);
		end_of_batch = req->end_of_batch;

		/* Remember batch list, for bulk free */
		req->next_id = next_id;
		next_id = req->id;

		/* Execute request */
		sim_execute_request(wctx, req->svc_req, req->svc_res,
				    req->cb, req->cb_ctx);

		osal_atomic_fetch_add(&q->num_processed, 1);

		if (is_worker_stopping(wctx->core_id)) {
			break;
		}
	}

	/* End of batch callback and free */
	if (req && end_of_batch) {
		if (req->poll_mode) {
			/* Delay callback and free until poll  */
			req->is_proc_done = true;
		} else {
			if (req->cb) {
				req->cb(req->cb_ctx, req->svc_res);
			}

			/* Restore entries to free list */
			req_free_q_batch_enqueue(req);
		}
	}
}

int pnso_sim_run_worker_loop(void *opaque)
{
	int core_id = (int) (uint64_t) opaque;
	struct sim_worker_ctx *wctx = sim_get_worker_ctx(core_id);
	struct sim_q *q;

	if (!wctx) {
		return EINVAL;
	}

	q = wctx->req_q;
	while (1) {
		if (is_worker_stopping(core_id)) {
			break;
		}

		sim_q_wait_for_batch_done(q, core_id);
		pnso_sim_run_worker_once(wctx);
	}

	return 0;
}

pnso_error_t sim_start_worker_thread(int core_id)
{
	pnso_error_t rc;
	struct sim_worker_ctx *wctx = sim_get_worker_ctx(core_id);

	if (!wctx) {
		return EINVAL;
	}
	if ((rc = osal_thread_create(wctx->worker,
				     pnso_sim_run_worker_loop,
				     (void *) (uint64_t) core_id)) == 0) {
		if ((rc = osal_thread_bind(wctx->worker, core_id)) == 0) {
			rc = osal_thread_start(wctx->worker);
		}
	}
	return rc;
}

pnso_error_t sim_stop_worker_thread(int core_id)
{
	struct sim_worker_ctx *wctx = sim_lookup_worker_ctx(core_id);

	if (!wctx) {
		return EINVAL;
	}
	return osal_thread_stop(wctx->worker);
}

