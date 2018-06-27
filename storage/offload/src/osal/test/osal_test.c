#ifndef __KERNEL__
#include <unistd.h>
#include <assert.h>
#include "pnso_sim_api.h"
#include "pnso_sim.h"
#endif
#include "osal_mem.h"
#include "osal_thread.h"
#include "osal_atomic.h"
#include "osal_setup.h"
#include "osal_log.h"
#include "osal_sys.h"
#include "osal_errno.h"

OSAL_LICENSE("Dual BSD/GPL");

#ifndef __KERNEL__

#define PNSO_TEST_DATA_SIZE 4*1024

struct pnso_buffer_list *src_buflist;
struct pnso_buffer_list *int_buflist;
struct pnso_buffer_list *dst_buflist;
uint8_t src_buf[PNSO_TEST_DATA_SIZE];
uint8_t int_buf[PNSO_TEST_DATA_SIZE];
uint8_t dst_buf[PNSO_TEST_DATA_SIZE];
osal_atomic_int_t cp_req_done;
osal_atomic_int_t dc_req_done;
struct pnso_service_request *svc_req;
struct pnso_service_result *svc_res;
osal_thread_t wafl_thread, poll_thread;

void cp_comp_cb(void *arg1, struct pnso_service_result *svc_res)
{
	osal_log("IO: CP request completed\n");
	osal_log("IO: compressed length is %d\n",
			svc_res->svc[0].u.dst.data_len);
	osal_atomic_set(&cp_req_done, 1);
}

int exec_cp_req(void)
{
	int rc;

	svc_req->sgl = src_buflist;

	/* Setup 1 services */
	svc_req->num_services = 1;
	svc_res->num_services = 1;

	/* Setup compression service */
	svc_req->svc[0].svc_type = PNSO_SVC_TYPE_COMPRESS;
	svc_req->svc[0].u.cp_desc.algo_type = PNSO_COMPRESSOR_TYPE_LZRW1A;
	svc_req->svc[0].u.cp_desc.flags = PNSO_DFLAG_ZERO_PAD | PNSO_DFLAG_INSERT_HEADER;
	svc_req->svc[0].u.cp_desc.threshold_len = PNSO_TEST_DATA_SIZE - 8;
	svc_res->svc[0].u.dst.sgl = int_buflist;

	rc = pnso_submit_request(PNSO_BATCH_REQ_NONE,
				svc_req, svc_res,
				cp_comp_cb, NULL,
				NULL, NULL);
	if (rc != 0) {
		osal_log("pnso_submit_request for cp failed with rc: %d\n", rc);
		return rc;
	}

	return 0;
}

void dc_comp_cb(void *arg1, struct pnso_service_result *svc_res)
{
	osal_log("IO: DC request completed\n");
	osal_log("IO: Decompressed length is %d\n",
			svc_res->svc[0].u.dst.data_len);
	osal_atomic_set(&dc_req_done, 1);
}

int exec_dc_req(void)
{
	int rc;

	svc_req->sgl = int_buflist;

	/* Setup 1 services */
	svc_req->num_services = 1;
	svc_res->num_services = 1;

	/* Setup compression service */
	svc_req->svc[0].svc_type = PNSO_SVC_TYPE_DECOMPRESS;
	svc_req->svc[0].u.dc_desc.algo_type = PNSO_COMPRESSOR_TYPE_LZRW1A;
	svc_req->svc[0].u.dc_desc.flags = PNSO_DFLAG_HEADER_PRESENT;
	svc_res->svc[0].u.dst.sgl = dst_buflist;

	rc = pnso_submit_request(PNSO_BATCH_REQ_NONE,
				svc_req, svc_res,
				dc_comp_cb, NULL,
				NULL, NULL);
	if (rc != 0) {
		osal_log("pnso_submit_request for dc failed with rc: %d\n", rc);
		return rc;
	}

	return 0;

}

int exec_req(void *arg)
{
	size_t alloc_sz;
	/* Allocate request and response */
	alloc_sz = sizeof(struct pnso_service_request) + PNSO_SVC_TYPE_MAX*sizeof(struct pnso_service);
	svc_req = (struct pnso_service_request *) osal_alloc(alloc_sz);
	memset(svc_req, 0, alloc_sz);

	alloc_sz = sizeof(struct pnso_service_result) + PNSO_SVC_TYPE_MAX*sizeof(struct pnso_service_status);
	svc_res = (struct pnso_service_result *) osal_alloc(alloc_sz);
	memset(svc_res, 0, alloc_sz);

	osal_atomic_init(&cp_req_done, 0);
	osal_atomic_init(&dc_req_done, 0);

	/* Prep the data */
	memset(src_buf, 3, sizeof(src_buf));
	memset(int_buf, 0, sizeof(int_buf));
	memset(dst_buf, 0, sizeof(dst_buf));

	src_buflist = (struct pnso_buffer_list *)osal_alloc(sizeof(struct pnso_buffer_list));
	int_buflist = (struct pnso_buffer_list *)osal_alloc(sizeof(struct pnso_buffer_list));
	dst_buflist = (struct pnso_buffer_list *)osal_alloc(sizeof(struct pnso_buffer_list));

	src_buflist->count = 1;
	src_buflist->buffers[0].buf = (uint64_t)src_buf;
	src_buflist->buffers[0].len = sizeof(src_buf);
	int_buflist->count = 1;
	int_buflist->buffers[0].buf = (uint64_t)int_buf;
	int_buflist->buffers[0].len = sizeof(int_buf);
	dst_buflist->count = 1;
	dst_buflist->buffers[0].buf = (uint64_t)dst_buf;
	dst_buflist->buffers[0].len = sizeof(dst_buf);

	/* Prep the polling thread */
	pnso_sim_thread_init();

	exec_cp_req();
	while (1) {
		int cp_done = osal_atomic_read(&cp_req_done);

		if (!cp_done) {
			usleep(10000);
		} else {
			break;
		}
	}
	exec_dc_req();
	return 0;
}
#endif

#define MAX_NUM_THREADS 128
uint8_t thread_id_arr[MAX_NUM_THREADS];
osal_atomic_int_t thread_done[MAX_NUM_THREADS];
static int nthreads;
int thread_test_fn(void* arg) 
{
	int core = osal_get_coreid();
	int id = (int)((uint64_t)arg);

	thread_id_arr[nthreads++] = core;
#ifndef __KERNEL__
	assert(core == osal_get_coreid());
#endif
	if (core != osal_get_coreid())
	{
		osal_err("Core id mismatch\n");
		return EINVAL;
	}
	osal_atomic_set(&thread_done[id], 1);
	return 0;
}

osal_thread_t ot[MAX_NUM_THREADS];
int osal_thread_test(void)
{
	int done = 0;
	void *arg = NULL;
	int i, rv;
	int max_threads = osal_get_core_count();

	for (i = 0; i < max_threads; i++)
	{
		arg = (void *)((uint64_t)i);
		rv = osal_thread_run(&ot[i], thread_test_fn, arg);
		if(rv != 0)
		{
			return rv;
		}
		do
		{
			done = osal_atomic_read(&thread_done[i]);
			osal_yield();
		} while (done != 1);
	}
	for (i = 0; i < max_threads; i++)
	{
		rv = osal_thread_stop(&ot[i]);
		if(rv != 0)
		{
			return rv;
		}
#ifndef __KERNEL__
		assert(thread_id_arr[i] == i);
#endif
	}
	return 0;
}

int body(void)
{
	int rv;
#ifndef __KERNEL__
	struct pnso_init_params init_params;

	memset(&init_params, 0, sizeof(init_params));
	/* Initialize session */
	init_params.cp_hdr_version = 1;
	init_params.per_core_qdepth = 16;
	init_params.block_size = 4096;
	pnso_init(&init_params);
	osal_thread_run(&wafl_thread, exec_req, NULL);
	while (1) {
		int cp_done = osal_atomic_read(&cp_req_done);
		int dc_done = osal_atomic_read(&dc_req_done);

		if (!cp_done || !dc_done) {
			usleep(10000);
		} else {
			break;
		}
	}
	if (memcmp(src_buf, dst_buf, sizeof(src_buf)) == 0) {
		osal_log("IO: Final memcmp passed\n");
	} else {
		osal_log("IO: Final memcmp failed\n");
		return EINVAL;
	}
#endif
	rv = osal_thread_test();
	if(rv == 0)
	{
		osal_log("PNSO: Osal test complete\n");
	}
	return rv;
}

osal_init_fn_t init_fp;
osal_fini_fn_t fini_fp;
OSAL_SETUP(init_fp, body, fini_fp);
