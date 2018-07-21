/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <gtest/gtest.h>
#include <vector>
#include <unistd.h>

#include "osal_logger.h"
#include "pnso_api.h"

#include "pnso_pbuf.h"
#include "pnso_mpool.h"
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"

/*
 * NOTE:
 * 	Some or most of the UTs may appear to be repeatitive with minor changes
 * 	and thereby the functions may be long, and this is deliberate for the
 * 	following reasons:
 * 		(a) to keep the relevant pieces of a UT within the vicinity
 * 		(b) to keep up the readability
 * 		(c) to ease the troubleshooting
 *
 * 	Code optimization is not the concern.
 *
 * 	TODO-hash:
 * 		- make PNSO_BLOCK_SIZE visible via config-get, when pnso_chain.c
 * 		comes into play
 */
#define PNSO_BLOCK_SIZE		4096

using namespace std;

class pnso_hash_test : public ::testing::Test {
public:

protected:

    pnso_hash_test() {
    }

    virtual ~pnso_hash_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

static inline uint32_t
get_block_len(const struct pnso_flat_buffer *buf, uint32_t block_idx,
		uint32_t block_size)
{
	uint32_t len;

	if (buf->len >= (block_size * (block_idx + 1)))
		len = block_size;
	else if (buf->len >= (block_size * block_idx))
		len = buf->len % block_size;
	else
		len = 0;

	return len;
}

static inline uint32_t
get_cpdc_desc_object_size(void)
{
	uint32_t pad_size;

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_desc),
			PNSO_MEM_ALIGN_DESC);
	return (sizeof(struct cpdc_desc) + pad_size);
}

static inline uint32_t
get_cpdc_status_desc_object_size(void)
{
	uint32_t pad_size;

	pad_size = mpool_get_pad_size(sizeof(struct cpdc_status_desc),
			PNSO_MEM_ALIGN_DESC);
	return (sizeof(struct cpdc_status_desc) + pad_size);
}

static void
ut_hash_setup_buffer(void) {
	pnso_error_t err;
	struct cpdc_init_params init_params;
	struct pnso_hash_desc pnso_hash_desc;
	struct service_params svc_params;
	struct service_info svc_info;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct cpdc_desc *hash_desc;
	struct cpdc_status_desc *status_desc;
	uint32_t len, count;
	uint32_t temp_len;
	uint16_t algo_type;

	OSAL_LOG_INFO("enter ...");

	init_params.cip_version = 0x1234;
	init_params.cip_qdepth = 16;
	init_params.cip_block_size = PNSO_BLOCK_SIZE;

	err = cpdc_start_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	/* use this same setup across UTs */
	count = 1;
	len = 32;

	/* init service params */
	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);

	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
	pnso_hash_desc.flags = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_hash_desc = &pnso_hash_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_HASH;
	svc_info.si_flags = CHAIN_SFLAG_LONE_SERVICE;
	svc_info.si_ops = hash_ops;

	/* ------------------------------------------------------------------ */
	OSAL_LOG_INFO("=== verify with NULL service info");
	err = svc_info.si_ops.setup(NULL, &svc_params);
	EXPECT_EQ(err, EINVAL);
	OSAL_LOG_INFO("=== verify with NULL service params");
	err = svc_info.si_ops.setup(&svc_info, NULL);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify with invalid src buf list len");
	temp_len = svc_params.sp_src_blist->buffers[0].len;
	svc_params.sp_src_blist->buffers[0].len = 0;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	/* restore to original */
	svc_params.sp_src_blist->buffers[0].len = temp_len;

	OSAL_LOG_INFO("=== verify invalid algo type");
	algo_type = pnso_hash_desc.algo_type;
	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_NONE;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_MAX;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	pnso_hash_desc.algo_type = algo_type;	/* restore to original */

	OSAL_LOG_INFO("=== verify valid case with 512 byte hash ");
	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_NE(svc_info.si_src_sgl, nullptr);
	EXPECT_NE(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);
	EXPECT_EQ(hash_desc->u.cd_bits.cc_src_is_list, 1);
	EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_enabled, 1);
	/* PNSO_HASH_TYPE_SHA2_512 = 0, PNSO_HASH_TYPE_SHA2_256 = 1 */
	EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_type, 0);
	len = pbuf_get_buffer_list_len(src_blist);
	EXPECT_EQ(hash_desc->cd_datain_len, len);
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with 256 byte hash ");
	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_256;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_NE(svc_info.si_src_sgl, nullptr);
	EXPECT_NE(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);
	EXPECT_EQ(hash_desc->u.cd_bits.cc_src_is_list, 1);
	EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_enabled, 1);
	/* PNSO_HASH_TYPE_SHA2_512 = 0, PNSO_HASH_TYPE_SHA2_256 = 1 */
	EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_type, 1);
	len = pbuf_get_buffer_list_len(src_blist);
	EXPECT_EQ(hash_desc->cd_datain_len, len);
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify flag(s)");
	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_256;
	pnso_hash_desc.flags = 0;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	EXPECT_EQ(hash_desc->u.cd_bits.cc_src_is_list, 1);
	/* ------------------------------------------------------------------ */

	cpdc_stop_accelerator();
}

void ut_hash_setup_per_block(void) {
	pnso_error_t err;
	struct cpdc_init_params init_params;
	struct pnso_hash_desc pnso_hash_desc;
	struct service_params svc_params;
	struct service_info svc_info;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct pnso_flat_buffer *interm_fbuf;
	struct cpdc_desc *hash_desc;
	struct cpdc_status_desc *status_desc;
	uint32_t block_size, block_len;
	uint32_t block_count, desc_object_size, status_desc_object_size;
	uint32_t i, len, count, temp_len;
	uint16_t algo_type;
	char *object;

	OSAL_LOG_INFO("enter ...");

	init_params.cip_version = 0x1234;
	init_params.cip_qdepth = 16;
	init_params.cip_block_size = PNSO_BLOCK_SIZE;

	err = cpdc_start_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	/* use this same setup across UTs */
	count = 1;
	len = 32;
	block_size = init_params.cip_block_size; 
	block_count = (interm_fbuf->len + (block_size - 1)) / block_size;
	desc_object_size = get_cpdc_desc_object_size();
	status_desc_object_size = get_cpdc_status_desc_object_size();

	/* init service params */
	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);

#define PNSO_BUFFER_LEN	(32 * 1024)	/* TODO-hash_ut: move this out */
	len = PNSO_BUFFER_LEN;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_params.u.sp_hash_desc = &pnso_hash_desc;

	/* init caller's hash descriptor */
	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
	pnso_hash_desc.flags = PNSO_HASH_DFLAG_PER_BLOCK;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_HASH;
	svc_info.si_flags = CHAIN_SFLAG_LONE_SERVICE;
	svc_info.si_block_size = block_size;
	svc_info.si_ops = hash_ops;
	svc_info.si_interm_fbuf = interm_fbuf;

	/* ------------------------------------------------------------------ */
	OSAL_LOG_INFO("=== verify with NULL service info");
	err = svc_info.si_ops.setup(NULL, &svc_params);
	EXPECT_EQ(err, EINVAL);
	OSAL_LOG_INFO("=== verify with NULL service params");
	err = svc_info.si_ops.setup(&svc_info, NULL);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify with invalid src buf list len");
	temp_len = svc_params.sp_src_blist->buffers[0].len;
	svc_params.sp_src_blist->buffers[0].len = 0;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	/* restore to original */
	svc_params.sp_src_blist->buffers[0].len = temp_len;

	OSAL_LOG_INFO("=== verify invalid algo type");
	algo_type = pnso_hash_desc.algo_type;
	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_NONE;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_MAX;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	pnso_hash_desc.algo_type = algo_type;	/* restore to original */
	
	OSAL_LOG_INFO("=== verify flag(s) - per_block");
	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_256;
	pnso_hash_desc.flags = PNSO_HASH_DFLAG_PER_BLOCK;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	EXPECT_EQ(hash_desc->u.cd_bits.cc_src_is_list, 0);
	
	OSAL_LOG_INFO("=== verify valid case with 512 byte hash ");
	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(hash_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_enabled, 1);
		/* PNSO_HASH_TYPE_SHA2_512 = 0, PNSO_HASH_TYPE_SHA2_256 = 1 */
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_type, 0);
		EXPECT_EQ(hash_desc->cd_datain_len, block_len);

		object = (char *) hash_desc;
		object += desc_object_size;
		hash_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with 512-byte hash, data buffer equals to 4KB");
	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BLOCK_SIZE;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(hash_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_enabled, 1);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_type, 0);
		EXPECT_EQ(hash_desc->cd_datain_len, block_len);

		object = (char *) hash_desc;
		object += desc_object_size;
		hash_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with 512-byte hash, data buffer greater than 4KB");
	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BLOCK_SIZE + 1;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(hash_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_enabled, 1);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_type, 0);
		EXPECT_EQ(hash_desc->cd_datain_len, block_len);

		object = (char *) hash_desc;
		object += desc_object_size;
		hash_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with 512-byte hash, data buffer less than 4KB");
	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BLOCK_SIZE - 1;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(hash_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_enabled, 1);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_type, 0);
		EXPECT_EQ(hash_desc->cd_datain_len, block_len);

		object = (char *) hash_desc;
		object += desc_object_size;
		hash_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with 256 byte hash ");
	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_256;

	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BUFFER_LEN;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(hash_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_enabled, 1);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_type, 1);
		EXPECT_EQ(hash_desc->cd_datain_len, block_len);

		object = (char *) hash_desc;
		object += desc_object_size;
		hash_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with 256-byte hash, data buffer equals to 4KB");
	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BLOCK_SIZE;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(hash_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_enabled, 1);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_type, 1);
		EXPECT_EQ(hash_desc->cd_datain_len, block_len);

		object = (char *) hash_desc;
		object += desc_object_size;
		hash_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with 256-byte hash, data buffer greather than 4KB");
	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BLOCK_SIZE + 1;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(hash_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_enabled, 1);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_type, 1);
		EXPECT_EQ(hash_desc->cd_datain_len, block_len);

		object = (char *) hash_desc;
		object += desc_object_size;
		hash_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with 256-byte hash, data buffer less than 4KB");
	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BLOCK_SIZE - 1;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(hash_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_enabled, 1);
		EXPECT_EQ(hash_desc->u.cd_bits.cc_hash_type, 1);
		EXPECT_EQ(hash_desc->cd_datain_len, block_len);

		object = (char *) hash_desc;
		object += desc_object_size;
		hash_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);
	/* ------------------------------------------------------------------ */

	cpdc_stop_accelerator();
}

TEST_F(pnso_hash_test, ut_hash_setup) {
	/* UT for entire buffer */
	ut_hash_setup_buffer();

	/* UT for per-block buffer */
	ut_hash_setup_per_block();
}

TEST_F(pnso_hash_test, ut_hash_chain) {
	/* TODO-hash_ut: ... */
}

TEST_F(pnso_hash_test, ut_hash_schedule) {
	pnso_error_t err;
	struct cpdc_init_params init_params;
	struct pnso_hash_desc pnso_hash_desc;
	struct service_params svc_params;
	struct service_info svc_info;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	uint32_t len, count;

	OSAL_LOG_INFO("enter ...");

	init_params.cip_version = 0x1234;
	init_params.cip_qdepth = 16;
	init_params.cip_block_size = PNSO_BLOCK_SIZE;

	err = cpdc_start_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	/* use this same setup across UTs */
	count = 1;
	len = 32;

	/* init service params */
	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);

	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
	pnso_hash_desc.flags = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_hash_desc = &pnso_hash_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_HASH;
	svc_info.si_flags = 0xF;
	svc_info.si_ops = hash_ops;

	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	/* ------------------------------------------------------------------ */
	OSAL_LOG_INFO("=== verify invalid service flag");
	svc_info.si_flags = 0;
	err = svc_info.si_ops.schedule(&svc_info);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify lone service flag");
	svc_info.si_flags = CHAIN_SFLAG_LONE_SERVICE;
	err = svc_info.si_ops.schedule(&svc_info);
	EXPECT_EQ(err, PNSO_OK);

	OSAL_LOG_INFO("=== verify first service flag");
	svc_info.si_flags = CHAIN_SFLAG_FIRST_SERVICE;
	err = svc_info.si_ops.schedule(&svc_info);
	EXPECT_EQ(err, PNSO_OK);

	OSAL_LOG_INFO("=== verify lone and/or first service flag");
	svc_info.si_flags = (CHAIN_SFLAG_LONE_SERVICE |
			CHAIN_SFLAG_FIRST_SERVICE);
	err = svc_info.si_ops.schedule(&svc_info);
	EXPECT_EQ(err, PNSO_OK);

	OSAL_LOG_INFO("=== verify non-lone or first service flag");
	svc_info.si_flags = 0xF;
	svc_info.si_flags &= ~(CHAIN_SFLAG_LONE_SERVICE |
			CHAIN_SFLAG_FIRST_SERVICE);
	OSAL_LOG_INFO("=== flag: %x", svc_info.si_flags);
	err = svc_info.si_ops.schedule(&svc_info);
	EXPECT_EQ(err, EINVAL);
	/* ------------------------------------------------------------------ */

	svc_info.si_ops.teardown(&svc_info);
	cpdc_stop_accelerator();
}

TEST_F(pnso_hash_test, ut_hash_poll) {
	/* TODO-hash_ut: ... */
}

TEST_F(pnso_hash_test, ut_hash_read_status) {
	pnso_error_t err;
	struct cpdc_init_params init_params;
	struct pnso_hash_desc pnso_hash_desc;
	struct service_params svc_params;
	struct service_info svc_info;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct cpdc_desc *hash_desc;
	struct cpdc_status_desc *status_desc;
	uint32_t len, count;

	OSAL_LOG_INFO("enter ...");

	init_params.cip_version = 0x1234;
	init_params.cip_qdepth = 16;
	init_params.cip_block_size = PNSO_BLOCK_SIZE;

	err = cpdc_start_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	/* use this same setup across UTs */
	count = 1;
	len = 32;

	/* init service params */
	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);

	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
	pnso_hash_desc.flags = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_hash_desc = &pnso_hash_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_HASH;
	svc_info.si_flags = 0xF;
	svc_info.si_ops = hash_ops;

	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	/* get cp desc and status descriptor */
	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	/* init status descriptor */
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 0;
	// status_desc->csd_output_data_len = hash_desc->cd_datain_len;
	status_desc->csd_partial_data = hash_desc->cd_status_data;
	status_desc->csd_integrity_data = 0xffff1234eeee5678;

	/* ------------------------------------------------------------------ */
	OSAL_LOG_INFO("=== verify valid bit not set");
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 0;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify hw error set");
	status_desc->csd_err = 0x1;	/* fake the error */
	status_desc->csd_valid = 1;
	status_desc->csd_partial_data = 0;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify status and partial data mismatch");
	status_desc->csd_err = 0x1; 	/* fake the error */
	status_desc->csd_valid = 1;
	status_desc->csd_partial_data = hash_desc->cd_status_data + 1;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify integrity data not set");
	status_desc->csd_err = 0x1; 	/* fake the error */
	status_desc->csd_valid = 1;
	status_desc->csd_partial_data = hash_desc->cd_status_data;
	// status_desc->csd_integrity_data = 0;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify valid case");
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 1;
	// status_desc->csd_output_data_len = hash_desc->cd_datain_len;
	status_desc->csd_partial_data = hash_desc->cd_status_data;
	// status_desc->csd_integrity_data = 0xffff1234eeee5678;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, PNSO_OK);
	/* ------------------------------------------------------------------ */

	svc_info.si_ops.teardown(&svc_info);
	cpdc_stop_accelerator();
}

TEST_F(pnso_hash_test, ut_hash_write_result) {
#define  ENABLE_WRITE_RESULT 0
#if ENABLE_WRITE_RESULT
	pnso_error_t err;
	struct cpdc_init_params init_params;
	struct pnso_hash_desc pnso_hash_desc;
	struct service_params svc_params;
	struct service_info svc_info;
	struct pnso_service_status svc_status;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct cpdc_desc *hash_desc;
	struct cpdc_status_desc *status_desc;
	uint32_t len, count;

	OSAL_LOG_INFO("enter ...");

	init_params.cip_version = 0x1234;
	init_params.cip_qdepth = 16;
	init_params.cip_block_size = PNSO_BLOCK_SIZE;

	err = cpdc_start_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	/* use this same setup across UTs */
	count = 1;
	len = 32;

	/* init service params */
	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);

	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
	pnso_hash_desc.flags = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_hash_desc = &pnso_hash_desc;

	/* init service status */
	memset(&svc_status, 0, sizeof(struct pnso_service_status));
	svc_status.err = EINVAL;
	svc_status.svc_type = PNSO_SVC_TYPE_HASH;
	svc_status.u.dst.data_len = 0;
	svc_status.u.dst.sgl = dst_blist;	/* use same buffer for target */

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_HASH;
	svc_info.si_flags = 0xF;
	svc_info.si_ops = hash_ops;
	svc_info.si_svc_status  = &svc_status;

	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	/* get cp desc and status descriptor */
	hash_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(hash_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	/* init status descriptor */
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 0;
	// status_desc->csd_output_data_len = hash_desc->cd_datain_len;
	status_desc->csd_partial_data = hash_desc->cd_status_data;
	// status_desc->csd_integrity_data = 0xffff1234eeee5678;

	/* ------------------------------------------------------------------ */
	OSAL_LOG_INFO("=== verify valid bit not set");
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 0;
	svc_status.err = PNSO_OK;
	err = svc_info.si_ops.write_result(&svc_info);
	EXPECT_EQ(svc_status.err, EINVAL);

	OSAL_LOG_INFO("=== verify hw error set");
	status_desc->csd_err = 0x1;	/* fake the error */
	status_desc->csd_valid = 1;
	svc_status.err = PNSO_OK;
	err = svc_info.si_ops.write_result(&svc_info);
	EXPECT_NE(svc_status.err, PNSO_OK);

	OSAL_LOG_INFO("=== verify valid case");
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 1;
	status_desc->csd_partial_data = hash_desc->cd_status_data;
	svc_status.u.dst.data_len = 0;
	err = svc_info.si_ops.write_result(&svc_info);
	EXPECT_EQ(err, PNSO_OK);
	EXPECT_NE(svc_status.u.dst.data_len, 0);
	/* ------------------------------------------------------------------ */

	svc_info.si_ops.teardown(&svc_info);
	cpdc_stop_accelerator();
#endif
}

TEST_F(pnso_hash_test, ut_hash_teardown) {
	pnso_error_t err;
	struct cpdc_init_params init_params;
	struct pnso_hash_desc pnso_hash_desc;
	struct service_params svc_params;
	struct service_info svc_info;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct pnso_flat_buffer *interm_fbuf;
	uint32_t block_size;
	uint32_t len, count;
	uint16_t temp_flags;

	OSAL_LOG_INFO("enter ...");

	init_params.cip_version = 0x1234;
	init_params.cip_qdepth = 16;
	init_params.cip_block_size = PNSO_BLOCK_SIZE;

	err = cpdc_start_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	/* use this same setup across UTs */
	count = 1;
	len = 32;
	block_size = init_params.cip_block_size; 

	/* init service params */
	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);

#define PNSO_BUFFER_LEN	(32 * 1024)	/* TODO-hash_ut: move this out */
	len = PNSO_BUFFER_LEN;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_params.u.sp_hash_desc = &pnso_hash_desc;

	/* init caller's hash descriptor */
	pnso_hash_desc.algo_type = PNSO_HASH_TYPE_SHA2_512;
	pnso_hash_desc.flags = 0;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_HASH;
	svc_info.si_flags = CHAIN_SFLAG_LONE_SERVICE;
	svc_info.si_ops = hash_ops;
	svc_info.si_interm_fbuf = interm_fbuf;

	/* ------------------------------------------------------------------ */
	OSAL_LOG_INFO("=== verify valid case non-per_block");
	temp_flags = pnso_hash_desc.flags;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);
	svc_info.si_ops.teardown(&svc_info);
	pnso_hash_desc.flags = temp_flags;	/* restore to original */

	OSAL_LOG_INFO("=== verify valid case per_block");
	svc_info.si_type = PNSO_SVC_TYPE_HASH;
	svc_info.si_flags = CHAIN_SFLAG_LONE_SERVICE;
	svc_info.si_block_size = block_size;
	svc_info.si_ops = hash_ops;
	svc_info.si_interm_fbuf = interm_fbuf;

	temp_flags = pnso_hash_desc.flags;
	pnso_hash_desc.flags = PNSO_HASH_DFLAG_PER_BLOCK;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);
	svc_info.si_ops.teardown(&svc_info);
	pnso_hash_desc.flags = temp_flags;	/* restore to original */

	OSAL_LOG_INFO("=== TODO-hash_ut: verify mpool count on cp desc/sgl/etc.");
	/* ------------------------------------------------------------------ */

	cpdc_stop_accelerator();
}

int main(int argc, char **argv) {
    int ret;
    osal_log_init(OSAL_LOG_LEVEL_INFO);
    ::testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    osal_log_deinit();
    return ret;
}
