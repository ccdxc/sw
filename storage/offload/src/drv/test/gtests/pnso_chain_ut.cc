/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <gtest/gtest.h>
#include <vector>
#include <unistd.h>

#include "osal_logger.h"
#include "osal_assert.h"
#include "pnso_api.h"

#include "pnso_pbuf.h"
#include "pnso_mpool.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"

#include "pnso_global_ut.hpp"

using namespace std;

extern struct mem_pool *svc_chain_mpool;
extern struct mem_pool *svc_chain_entry_mpool;

struct cpdc_init_params init_params;

struct pnso_buffer_list *src_blist;
struct pnso_buffer_list *dst_blist;
struct pnso_buffer_list *comp_blist;

class pnso_chain_test : public ::testing::Test {
public:

protected:

    pnso_chain_test() {
    }

    virtual ~pnso_chain_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

static void
dummy_completion_cb(void *cb_ctx, struct pnso_service_result *svc_res)
{
	EXPECT_NE(svc_res, nullptr);
	OSAL_LOG_INFO("completion cb fini");
}

static void
pprint_crypto_desc(struct pnso_crypto_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: %p", "=== crypto_desc", desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "rsvd", desc->rsvd);
	OSAL_LOG_INFO("%30s: %d", "key_desc_idx", desc->key_desc_idx);
	OSAL_LOG_INFO("%30s: %ju", "key_desc_idx", desc->iv_addr);
}

static void
pprint_cp_desc(struct pnso_compression_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: %p", "=== cp_desc", desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "flags", desc->flags);
	OSAL_LOG_INFO("%30s: %d", "threshold_len",
			desc->threshold_len);
	OSAL_LOG_INFO("%30s: %d", "hdr_fmt_idx", desc->hdr_fmt_idx);
}

static void
pprint_dc_desc(struct pnso_decompression_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: %p", "=== dc_desc", desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "flags", desc->flags);
	OSAL_LOG_INFO("%30s: %d", "hdr_fmt_idx", desc->hdr_fmt_idx);
	OSAL_LOG_INFO("%30s: %d", "rsvd", desc->rsvd);
}

static void
pprint_hash_desc(struct pnso_hash_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: %p", "=== hash_desc", desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "flags", desc->flags);
}

static void
pprint_chksum_desc(struct pnso_checksum_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: %p", "=== chksum_desc", desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->algo_type);
	OSAL_LOG_INFO("%30s: %d", "flags", desc->flags);
}

static void
pprint_decompaction_desc(struct pnso_decompaction_desc *desc)
{
	if (!desc)
		return;

	OSAL_LOG_INFO("%30s: %p", "=== decompact_desc", desc);

	OSAL_LOG_INFO("%30s: %d", "algo_type", desc->vvbn);
	OSAL_LOG_INFO("%30s: %d", "rsvc_1", desc->rsvd_1);
}

static void
req_pprint_request(struct pnso_service_request *req)
{
	uint32_t i;

	if (!req)
		return;

	OSAL_LOG_INFO("%30s: %p", "=== pnso_service_request", req);

	OSAL_LOG_INFO("%30s: %p", "=== sgl", req->sgl);
	pbuf_pprint_buffer_list(req->sgl);

	OSAL_LOG_INFO("%30s: %d", "num_services", req->num_services);
	for (i = 0; i < req->num_services; i++) {
		OSAL_LOG_INFO("%30s: %d", "service #", i+1);

		OSAL_LOG_INFO("%30s: %d", "svc_type", req->svc[i].svc_type);
		OSAL_LOG_INFO("%30s: %d", "rsvd", req->svc[i].rsvd);

		switch (req->svc[i].svc_type) {
		case PNSO_SVC_TYPE_ENCRYPT:
			pprint_crypto_desc(&req->svc[i].u.crypto_desc);
			break;
		case PNSO_SVC_TYPE_DECRYPT:
			pprint_crypto_desc(&req->svc[i].u.crypto_desc);
			break;
		case PNSO_SVC_TYPE_COMPRESS:
			pprint_cp_desc(&req->svc[i].u.cp_desc);
			break;
		case PNSO_SVC_TYPE_DECOMPRESS:
			pprint_dc_desc(&req->svc[i].u.dc_desc);
			break;
		case PNSO_SVC_TYPE_HASH:
			pprint_hash_desc(&req->svc[i].u.hash_desc);
			break;
		case PNSO_SVC_TYPE_CHKSUM:
			pprint_chksum_desc(&req->svc[i].u.chksum_desc);
			break;
		case PNSO_SVC_TYPE_DECOMPACT:
			pprint_decompaction_desc(
					&req->svc[i].u.decompact_desc);
			break;
		default:
			OSAL_ASSERT(0);
			break;
		}
	}
}

void
req_pprint_result(struct pnso_service_result *res)
{
	uint32_t i;

	if (!res)
		return;

	OSAL_LOG_INFO("%30s: %p", "=== pnso_service_result", res);

	OSAL_LOG_INFO("%30s: %d", "err", res->err);

	OSAL_LOG_INFO("%30s: %d", "num_services", res->num_services);
	for (i = 0; i < res->num_services; i++) {
		OSAL_LOG_INFO("%30s: %d", "service #", i+1);

		OSAL_LOG_INFO("%30s: %d", "err", res->svc[i].err);
		OSAL_LOG_INFO("%30s: %d", "svc_type", res->svc[i].svc_type);
		OSAL_LOG_INFO("%30s: %d", "rsvd_1", res->svc[i].rsvd_1);

		switch (res->svc[i].svc_type) {
		case PNSO_SVC_TYPE_ENCRYPT:
		case PNSO_SVC_TYPE_DECRYPT:
		case PNSO_SVC_TYPE_COMPRESS:
		case PNSO_SVC_TYPE_DECOMPRESS:
		case PNSO_SVC_TYPE_DECOMPACT:
			OSAL_LOG_INFO("%30s: %d", "data_len",
					res->svc[i].u.dst.data_len);
			pbuf_pprint_buffer_list(res->svc[i].u.dst.sgl);
			break;
		case PNSO_SVC_TYPE_HASH:
			OSAL_LOG_INFO("%30s: %d", "num_tags",
					res->svc[i].u.hash.num_tags);
			OSAL_LOG_INFO("%30s: %d", "rsvd_2",
					res->svc[i].u.hash.rsvd_2);
			/* TODO-hash: print SHA/tags */
			break;
		case PNSO_SVC_TYPE_CHKSUM:
			OSAL_LOG_INFO("%30s: %d", "num_tags",
					res->svc[i].u.chksum.num_tags);
			OSAL_LOG_INFO("%30s: %d", "rsvd_3",
					res->svc[i].u.chksum.rsvd_3);
			/* TODO-chksum: print chksum/tags */
			break;
		default:
			OSAL_ASSERT(0);
			break;
		}
	}
}

static void
ut_chain_pre(void)
{
	pnso_error_t err;
	struct cpdc_init_params init_params = { 0 };
	uint32_t len, count, num_objects;

	/* use this same setup across UTs */
	count = 1;
	len = 32;

	init_params.cip_version = 0x1234;
	init_params.cip_qdepth = 16;
	init_params.cip_block_size = PNSO_UT_BLOCK_SIZE;

	err = cpdc_start_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	num_objects = PNSO_NUM_OBJECTS;
	err = mpool_create(MPOOL_TYPE_SERVICE_CHAIN, num_objects,
			sizeof(struct service_chain), PNSO_MEM_ALIGN_DESC,
			&svc_chain_mpool);
	EXPECT_NE(svc_chain_mpool, nullptr);

	err = mpool_create(MPOOL_TYPE_SERVICE_CHAIN_ENTRY, num_objects,
			sizeof(struct chain_entry), PNSO_MEM_ALIGN_DESC,
			&svc_chain_entry_mpool);
	EXPECT_NE(svc_chain_entry_mpool, nullptr);

	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	pbuf_convert_buffer_list_v2p(src_blist);
	pbuf_pprint_buffer_list(src_blist);

	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_UT_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);
	pbuf_convert_buffer_list_v2p(dst_blist);
	pbuf_pprint_buffer_list(dst_blist);

	comp_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_UT_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);
	pbuf_convert_buffer_list_v2p(comp_blist);
	pbuf_pprint_buffer_list(comp_blist);
}

void
ut_chain_post(void)
{
	pbuf_free_buffer_list(comp_blist);
	pbuf_free_buffer_list(dst_blist);
	pbuf_free_buffer_list(src_blist);

	mpool_destroy(&svc_chain_entry_mpool);
	mpool_destroy(&svc_chain_mpool);

	cpdc_stop_accelerator();
}

static struct pnso_service_request *
create_service_request(uint32_t num_services,
				struct pnso_buffer_list *src_blist,
				struct pnso_service services[])
{
	struct pnso_service_request *req;
	struct pnso_service *svc;
	size_t num_bytes;
	uint32_t i;

	num_bytes = sizeof(struct pnso_service_request) +
	    num_services * sizeof(struct pnso_service);

	req = (struct pnso_service_request *) osal_alloc(num_bytes);
	EXPECT_NE(req, nullptr);

	req->sgl = src_blist;
	req->num_services = num_services;
	for (i = 0; i < num_services; i++) {
		svc = &services[i];
		memcpy(&req->svc[i], svc, sizeof(struct pnso_service));
	}

	OSAL_LOG_INFO("req created! req: %p num_services: %d",
			req, num_services);
	return req;
}

static void
destroy_service_request(struct pnso_service_request *req)
{
	EXPECT_NE(req, nullptr);

	OSAL_LOG_INFO("req destroyed! req: %p num_services: %d",
			req, req->num_services);

	osal_free(req);
}

static struct pnso_service_result *
create_service_result(uint32_t num_services,
		struct pnso_service_status status[])
{
	struct pnso_service_result *res;
	struct pnso_service_status *st;
	size_t num_bytes;
	uint32_t i;

	EXPECT_GT(num_services, 0);
	num_bytes = sizeof(struct pnso_service_result) +
	    num_services * sizeof(struct pnso_service_status);

	res = (struct pnso_service_result *) osal_alloc(num_bytes);
	EXPECT_NE(res, nullptr);

	res->err = EINVAL;
	res->num_services = num_services;
	for (i = 0; i < num_services; i++) {
		st = &status[i];
		memcpy(&res->svc[i], st, sizeof(struct pnso_service_status));
	}

	OSAL_LOG_INFO("result created! res: %p num_services: %d",
			res, num_services);
	return res;
}

static void
destroy_service_result(struct pnso_service_result *res)
{
	EXPECT_NE(res, nullptr);

	OSAL_LOG_INFO("result destroyed! req: %p num_services: %d",
			res, res->num_services);

	osal_free(res);
}

TEST_F(pnso_chain_test, ut_chn_build_chain) {
	/* TODO-chain_ut: ... */
}

static void
ut_chn_build_service_chain_basic(void)
{
	pnso_error_t err;
	uint32_t num_services;

	struct pnso_service services[2];
	struct pnso_service_status status[2];

	struct pnso_compression_desc cp_desc;
	struct pnso_service compress_svc;

	struct pnso_hash_desc hash_desc;
	struct pnso_service hash_svc;

	struct pnso_service_request *svc_req;
	struct pnso_service_result *svc_res;

	ut_chain_pre();

	/* setup caller's cp descriptor and create service */
	cp_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
	cp_desc.flags = PNSO_CP_DFLAG_INSERT_HEADER;

	compress_svc.svc_type = PNSO_SVC_TYPE_COMPRESS;
	memcpy(&compress_svc.u.cp_desc, &cp_desc,
			sizeof(struct pnso_compression_desc));

	/* setup caller's hash descriptor and create service */
	hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
	hash_desc.flags = PNSO_HASH_DFLAG_PER_BLOCK;

	hash_svc.svc_type = PNSO_SVC_TYPE_HASH;
	memcpy(&hash_svc.u.hash_desc, &hash_desc,
			sizeof(struct pnso_hash_desc));

	/* load services for request */
	num_services = 2;
	memcpy(&services[0], &compress_svc, sizeof(struct pnso_service));
	memcpy(&services[1], &hash_svc, sizeof(struct pnso_service));

	/* create service request */
	svc_req = create_service_request(num_services, src_blist, services);
	EXPECT_NE(svc_req, nullptr);

	status[0].err = EINVAL;
	status[0].svc_type = PNSO_SVC_TYPE_COMPRESS;
	status[0].u.dst.data_len = 0;
	status[0].u.dst.sgl = comp_blist;

	status[1].err = EINVAL;
	status[1].svc_type = PNSO_SVC_TYPE_HASH;
	status[1].u.hash.num_tags = 1;
	status[1].u.hash.tags =
		(struct pnso_hash_tag *) osal_alloc(status[1].u.hash.num_tags *
				sizeof(struct pnso_hash_tag));

	svc_res = create_service_result(num_services, status);
	EXPECT_NE(svc_res, nullptr);

	req_pprint_request(svc_req);
	req_pprint_result(svc_res);

	err = chn_build_chain(svc_req, svc_res, dummy_completion_cb,
			NULL, NULL, NULL);
	EXPECT_EQ(err, 0);

	destroy_service_result(svc_res);
	destroy_service_request(svc_req);

	ut_chain_post();
}

TEST_F(pnso_chain_test, ut_chn_build_service_chain) {
	ut_chn_build_service_chain_basic();
}

TEST_F(pnso_chain_test, ut_chn_execute_chain) {
	/* TODO-chain_ut: ... */
}

TEST_F(pnso_chain_test, ut_chn_destroy_chain) {
	/* TODO-chain_ut: ... */
}

int main(int argc, char **argv) {
    int ret;
    osal_log_init(OSAL_LOG_LEVEL_INFO);
    ::testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    osal_log_deinit();
    return ret;
}
