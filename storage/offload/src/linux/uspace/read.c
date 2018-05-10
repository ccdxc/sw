#include "offloader.h"

_Thread_local static bool sim_worker_inited;
int exec_read_thread(void *arg1, void *arg2)
{
	struct pnso_service_request *svc_req;
	struct pnso_service_result *svc_res;
	struct io_ctx *io = (struct io_ctx *)arg1;
	struct pnso_hash_or_chksum_tag* tags;
	char xts_iv[16] = "";
	size_t alloc_sz;
	int rc;

	if (arg1 == NULL) return PNSO_INVALID_ARG;
	printf("Servicing IO: %s in read thread\n", io->name);
	tags = io->tbuf[io->tchain.current_thread].tags;
	memset(tags, 0, PNSO_NUM_TAGS * sizeof(struct pnso_hash_or_chksum_tag));

	/* Allocate request and response */
	alloc_sz = sizeof(struct pnso_service_request) + PNSO_SVC_TYPE_MAX*sizeof(struct pnso_service);
	svc_req = (struct pnso_service_request *) malloc(alloc_sz);
	memset(svc_req, 0, alloc_sz);

	alloc_sz = sizeof(struct pnso_service_result) + PNSO_SVC_TYPE_MAX*sizeof(struct pnso_service_status);
	svc_res = (struct pnso_service_result *) malloc(alloc_sz);
	memset(svc_res, 0, alloc_sz);

	svc_req->src_buf = io->src_buflist[io->tchain.current_thread];
	svc_req->dst_buf = io->dst_buflist[io->tchain.current_thread];

	/* Setup 3 services */
	svc_req->num_services = 3;
	svc_res->num_services = 3;

	/* Setup hash service */
	svc_req->svc[0].svc_type = PNSO_SVC_TYPE_HASH;
	svc_req->svc[0].d.hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
	svc_res->svc[0].num_tags = 16;
	svc_res->svc[0].tags = tags;

	/* Setup encryption service */
	svc_req->svc[1].svc_type = PNSO_SVC_TYPE_DECRYPT;
	svc_req->svc[1].d.crypto_desc.key_desc_idx = 1;
	svc_req->svc[1].d.crypto_desc.iv_addr = (uint64_t) xts_iv;

	/* Setup decompression service */
	svc_req->svc[2].svc_type = PNSO_SVC_TYPE_DECOMPRESS;
	svc_req->svc[2].d.dc_desc.algo_type = PNSO_COMPRESSOR_TYPE_LZRW1A;
	svc_req->svc[2].d.dc_desc.flags = PNSO_DFLAG_HEADER_PRESENT;

	/* Start worker thread */
	if (!sim_worker_inited) {
		pnso_sim_thread_init();
		sim_worker_inited = true;
	}

	/* Execute synchronously */
	rc = pnso_submit_request(PNSO_BATCH_REQ_NONE, svc_req, svc_res, NULL, NULL, NULL, NULL);
	if (rc != 0) {
		printf("pnso_submit_request failed in read thread with rc: %d\n", rc);
		return rc;
	}

	spdk_event_call(spdk_event_allocate(
			io->tchain.threads[0],
			completion_fn, io, NULL));
	return 0;
}
