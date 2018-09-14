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

#include "pnso_global_ut.hpp"

using namespace std;

class pnso_chksum_test : public ::testing::Test {
public:

protected:

    pnso_chksum_test() {
    }

    virtual ~pnso_chksum_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

/* TODO-chksum: reuse these */
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
ut_chksum_setup_buffer(void) {
	pnso_error_t err;
	struct cpdc_init_params init_params;
	struct pnso_checksum_desc pnso_chksum_desc;
	struct service_params svc_params;
	struct service_info svc_info;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct cpdc_desc *chksum_desc;
	struct cpdc_status_desc *status_desc;
	uint32_t len, count;
	uint32_t temp_len;
	uint16_t algo_type;

	OSAL_LOG_DEBUG("enter ...");

	init_params.cip_version = 0x1234;
	init_params.cip_qdepth = 16;
	init_params.cip_block_size = PNSO_BLOCK_SIZE;

	err = cpdc_init_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	/* use this same setup across UTs */
	count = 1;
	len = 32;

	/* init service params */
	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	pbuf_convert_buffer_list_v2p(src_blist);
	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);
	pbuf_convert_buffer_list_v2p(dst_blist);

	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_MCRC64;
	pnso_chksum_desc.flags = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_chksum_desc = &pnso_chksum_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_CHKSUM;
	svc_info.si_flags = CHAIN_SFLAG_LONE_SERVICE;
	svc_info.si_ops = chksum_ops;

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
	svc_params.sp_src_blist->buffers[0].len = temp_len; /* restore original */

	OSAL_LOG_INFO("=== verify invalid algo type");
	algo_type = pnso_chksum_desc.algo_type;
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_NONE;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_MAX;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	pnso_chksum_desc.algo_type = algo_type;	/* restore original */

	OSAL_LOG_INFO("=== verify valid case with MCRC64 checksum ");
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_MCRC64;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_NE(svc_info.si_src_sgl, nullptr);
	EXPECT_NE(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 1);
	/* 1 for compressed, 0 for uncompressed buffer */
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_src, 1);
	/* PNSO_CHKSUM_TYPE_MCRC64 = 1, PNSO_CHKSUM_TYPE_CRC32C = 2 */
	/* PNSO_CHKSUM_TYPE_ADLER32 = 3, PNSO_CHKSUM_TYPE_MADLER32 = 4 */
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_type, 1);
	len = pbuf_get_buffer_list_len(src_blist);
	EXPECT_EQ(chksum_desc->cd_datain_len, len);
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with CRC32C checksum ");
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_CRC32C;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_NE(svc_info.si_src_sgl, nullptr);
	EXPECT_NE(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 1);
	/* 1 for compressed, 0 for uncompressed buffer */
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_src, 1);
	/* PNSO_CHKSUM_TYPE_MCRC64 = 1, PNSO_CHKSUM_TYPE_CRC32C = 2 */
	/* PNSO_CHKSUM_TYPE_ADLER32 = 3, PNSO_CHKSUM_TYPE_MADLER32 = 4 */
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_type, 2);
	len = pbuf_get_buffer_list_len(src_blist);
	EXPECT_EQ(chksum_desc->cd_datain_len, len);
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with ADLER32 checksum ");
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_ADLER32;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_NE(svc_info.si_src_sgl, nullptr);
	EXPECT_NE(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 1);
	/* 1 for compressed, 0 for uncompressed buffer */
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_src, 1);
	/* PNSO_CHKSUM_TYPE_MCRC64 = 1, PNSO_CHKSUM_TYPE_CRC32C = 2 */
	/* PNSO_CHKSUM_TYPE_ADLER32 = 3, PNSO_CHKSUM_TYPE_MADLER32 = 4 */
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_type, 3);
	len = pbuf_get_buffer_list_len(src_blist);
	EXPECT_EQ(chksum_desc->cd_datain_len, len);
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with MADLER32 checksum ");
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_MADLER32;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_NE(svc_info.si_src_sgl, nullptr);
	EXPECT_NE(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 1);
	/* 1 for compressed, 0 for uncompressed buffer */
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_src, 1);
	/* PNSO_CHKSUM_TYPE_MCRC64 = 1, PNSO_CHKSUM_TYPE_CRC32C = 2 */
	/* PNSO_CHKSUM_TYPE_ADLER32 = 3, PNSO_CHKSUM_TYPE_MADLER32 = 4 */
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_type, 4);
	len = pbuf_get_buffer_list_len(src_blist);
	EXPECT_EQ(chksum_desc->cd_datain_len, len);
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify flag(s)");
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_CRC32C;
	pnso_chksum_desc.flags = 0;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 1);
	/* ------------------------------------------------------------------ */

	cpdc_deinit_accelerator();
}

void ut_chksum_setup_per_block(void) {
	pnso_error_t err;
	struct cpdc_init_params init_params;
	struct pnso_checksum_desc pnso_chksum_desc;
	struct service_params svc_params;
	struct service_info svc_info;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct pnso_flat_buffer *interm_fbuf;
	struct cpdc_desc *chksum_desc;
	struct cpdc_status_desc *status_desc;
	uint32_t block_size, block_len;
	uint32_t block_count, desc_object_size, status_desc_object_size;
	uint32_t i, len, count, temp_len;
	uint16_t algo_type;
	char *object;

	OSAL_LOG_DEBUG("enter ...");

	init_params.cip_version = 0x1234;
	init_params.cip_qdepth = 16;
	init_params.cip_block_size = PNSO_BLOCK_SIZE;

	err = cpdc_init_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	/* use this same setup across UTs */
	count = 1;
	len = 32;
	block_size = init_params.cip_block_size; 
	desc_object_size = get_cpdc_desc_object_size();
	status_desc_object_size = get_cpdc_status_desc_object_size();

	/* init service params */
	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	pbuf_convert_buffer_list_v2p(src_blist);
	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);
	pbuf_convert_buffer_list_v2p(dst_blist);

	len = PNSO_BUFFER_LEN;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);
	pbuf_convert_flat_buffer_v2p(interm_fbuf);

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_params.u.sp_chksum_desc = &pnso_chksum_desc;

	/* init caller's chekcusm descriptor */
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_MCRC64;
	pnso_chksum_desc.flags = PNSO_CHKSUM_DFLAG_PER_BLOCK;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_CHKSUM;
	svc_info.si_flags = CHAIN_SFLAG_LONE_SERVICE;
	svc_info.si_block_size = block_size;
	svc_info.si_ops = chksum_ops;
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
	svc_params.sp_src_blist->buffers[0].len = temp_len; /* restore original */

	OSAL_LOG_INFO("=== verify invalid algo type");
	algo_type = pnso_chksum_desc.algo_type;
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_NONE;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_MAX;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	pnso_chksum_desc.algo_type = algo_type;	/* restore original */
	
	OSAL_LOG_INFO("=== verify flag(s) - per_block");
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_CRC32C;
	pnso_chksum_desc.flags = PNSO_CHKSUM_DFLAG_PER_BLOCK;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 0);
	
	OSAL_LOG_INFO("=== verify valid case with MCRC64 checksum ");
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_MCRC64;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	block_count = (interm_fbuf->len + (block_size - 1)) / block_size;
	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_src, 1);
		/* PNSO_CHKSUM_TYPE_MCRC64 = 1, PNSO_CHKSUM_TYPE_CRC32C = 2 */
		/* PNSO_CHKSUM_TYPE_ADLER32 = 3, PNSO_CHKSUM_TYPE_MADLER32 = 4 */
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_type, 1);
		EXPECT_EQ(chksum_desc->cd_datain_len, block_len);

		object = (char *) chksum_desc;
		object += desc_object_size;
		chksum_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with MCRC64 checksum, data buffer =4KB");
	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BLOCK_SIZE;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);
	pbuf_convert_flat_buffer_v2p(interm_fbuf);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	block_count = (interm_fbuf->len + (block_size - 1)) / block_size;
	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_src, 1);
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_type, 1);
		EXPECT_EQ(chksum_desc->cd_datain_len, block_len);

		object = (char *) chksum_desc;
		object += desc_object_size;
		chksum_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with MCRC64 checksum, data buffer >4KB");
	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BLOCK_SIZE + 1;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);
	pbuf_convert_flat_buffer_v2p(interm_fbuf);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	block_count = (interm_fbuf->len + (block_size - 1)) / block_size;
	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_src, 1);
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_type, 1);
		EXPECT_EQ(chksum_desc->cd_datain_len, block_len);

		object = (char *) chksum_desc;
		object += desc_object_size;
		chksum_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with MCRC64 checksum, data buffer <4KB");
	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BLOCK_SIZE - 1;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);
	pbuf_convert_flat_buffer_v2p(interm_fbuf);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	block_count = (interm_fbuf->len + (block_size - 1)) / block_size;
	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_src, 1);
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_type, 1);
		EXPECT_EQ(chksum_desc->cd_datain_len, block_len);

		object = (char *) chksum_desc;
		object += desc_object_size;
		chksum_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with CRC32C checksum ");
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_CRC32C;

	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BUFFER_LEN;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);
	pbuf_convert_flat_buffer_v2p(interm_fbuf);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	block_count = (interm_fbuf->len + (block_size - 1)) / block_size;
	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_src, 1);
		/* PNSO_CHKSUM_TYPE_MCRC64 = 1, PNSO_CHKSUM_TYPE_CRC32C = 2 */
		/* PNSO_CHKSUM_TYPE_ADLER32 = 3, PNSO_CHKSUM_TYPE_MADLER32 = 4 */
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_type, 2);
		EXPECT_EQ(chksum_desc->cd_datain_len, block_len);

		object = (char *) chksum_desc;
		object += desc_object_size;
		chksum_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with CRC32C checksum, data buffer =4KB");
	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BLOCK_SIZE;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);
	pbuf_convert_flat_buffer_v2p(interm_fbuf);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	block_count = (interm_fbuf->len + (block_size - 1)) / block_size;
	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_src, 1);
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_type, 2);
		EXPECT_EQ(chksum_desc->cd_datain_len, block_len);

		object = (char *) chksum_desc;
		object += desc_object_size;
		chksum_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with CRC32C checksum, data buffer >4KB");
	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BLOCK_SIZE + 1;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);
	pbuf_convert_flat_buffer_v2p(interm_fbuf);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	block_count = (interm_fbuf->len + (block_size - 1)) / block_size;
	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_src, 1);
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_type, 2);
		EXPECT_EQ(chksum_desc->cd_datain_len, block_len);

		object = (char *) chksum_desc;
		object += desc_object_size;
		chksum_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case with CRC32C checksum, data buffer <4KB");
	pbuf_free_flat_buffer(interm_fbuf);
	len = PNSO_BLOCK_SIZE - 1;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);
	pbuf_convert_flat_buffer_v2p(interm_fbuf);

	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_info.si_interm_fbuf = interm_fbuf;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_EQ(svc_info.si_src_sgl, nullptr);
	EXPECT_EQ(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);

	block_count = (interm_fbuf->len + (block_size - 1)) / block_size;
	for (i = 0; i < block_count; i++) {
		if (len <= PNSO_BLOCK_SIZE)
			block_len = len;
		else {
			block_len = PNSO_BLOCK_SIZE;
			len -= PNSO_BLOCK_SIZE;
		}
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_src_is_list, 0);
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_src, 1);
		/* PNSO_CHKSUM_TYPE_MCRC64 = 1, PNSO_CHKSUM_TYPE_CRC32C = 2 */
		/* PNSO_CHKSUM_TYPE_ADLER32 = 3, PNSO_CHKSUM_TYPE_MADLER32 = 4 */
		EXPECT_EQ(chksum_desc->u.cd_bits.cc_integrity_type, 2);
		EXPECT_EQ(chksum_desc->cd_datain_len, block_len);

		object = (char *) chksum_desc;
		object += desc_object_size;
		chksum_desc = (struct cpdc_desc *) object;

		object = (char *) status_desc;
		object += status_desc_object_size;
		status_desc = (struct cpdc_status_desc *) object;
	}
	svc_info.si_ops.teardown(&svc_info);

	/* TODO-chksum: cut reams */
	/* ------------------------------------------------------------------ */

	cpdc_deinit_accelerator();
}

TEST_F(pnso_chksum_test, ut_chksum_setup) {
	/* UT for entire buffer */
	ut_chksum_setup_buffer();

	/* UT for per-block buffer */
	ut_chksum_setup_per_block();
}

TEST_F(pnso_chksum_test, ut_chksum_chain) {
	/* TODO-chksum: ... */
}

TEST_F(pnso_chksum_test, ut_chksum_schedule) {
	pnso_error_t err;
	struct cpdc_init_params init_params;
	struct pnso_checksum_desc pnso_chksum_desc;
	struct service_params svc_params;
	struct service_info svc_info;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	uint32_t len, count;

	OSAL_LOG_DEBUG("enter ...");

	init_params.cip_version = 0x1234;
	init_params.cip_qdepth = 16;
	init_params.cip_block_size = PNSO_BLOCK_SIZE;

	err = cpdc_init_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	/* use this same setup across UTs */
	count = 1;
	len = 32;

	/* init service params */
	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	pbuf_convert_buffer_list_v2p(src_blist);
	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);
	pbuf_convert_buffer_list_v2p(dst_blist);

	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_MCRC64;
	pnso_chksum_desc.flags = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_chksum_desc = &pnso_chksum_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_CHKSUM;
	svc_info.si_flags = 0xF;
	svc_info.si_ops = chksum_ops;

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
	cpdc_deinit_accelerator();
}

TEST_F(pnso_chksum_test, ut_chksum_poll) {
	/* TODO-chksum: ... */
}

TEST_F(pnso_chksum_test, ut_chksum_read_status) {
	pnso_error_t err;
	struct cpdc_init_params init_params;
	struct pnso_checksum_desc pnso_chksum_desc;
	struct service_params svc_params;
	struct service_info svc_info;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct cpdc_desc *chksum_desc;
	struct cpdc_status_desc *status_desc;
	uint32_t len, count;

	OSAL_LOG_DEBUG("enter ...");

	init_params.cip_version = 0x1234;
	init_params.cip_qdepth = 16;
	init_params.cip_block_size = PNSO_BLOCK_SIZE;

	err = cpdc_init_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	/* use this same setup across UTs */
	count = 1;
	len = 32;

	/* init service params */
	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	pbuf_convert_buffer_list_v2p(src_blist);
	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);
	pbuf_convert_buffer_list_v2p(dst_blist);

	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_MCRC64;
	pnso_chksum_desc.flags = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_chksum_desc = &pnso_chksum_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_CHKSUM;
	svc_info.si_flags = 0xF;
	svc_info.si_ops = chksum_ops;

	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	/* get cp desc and status descriptor */
	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	/* init status descriptor */
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 0;
	status_desc->csd_partial_data = chksum_desc->cd_status_data;
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
	EXPECT_EQ(err, 0x1);

	OSAL_LOG_INFO("=== verify status and partial data mismatch");
	status_desc->csd_err = 0;
	status_desc->csd_valid = 1;
	status_desc->csd_partial_data = chksum_desc->cd_status_data + 1;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, EINVAL);

#if 0	/* TODO-chksum: */
	OSAL_LOG_INFO("=== verify integrity data not set");
	status_desc->csd_err = 0;
	status_desc->csd_valid = 1;
	status_desc->csd_partial_data = chksum_desc->cd_status_data;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, EINVAL);
#endif

	OSAL_LOG_INFO("=== verify valid case");
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 1;
	status_desc->csd_partial_data = chksum_desc->cd_status_data;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, PNSO_OK);
	/* ------------------------------------------------------------------ */

	svc_info.si_ops.teardown(&svc_info);
	cpdc_deinit_accelerator();
}

TEST_F(pnso_chksum_test, ut_chksum_write_result) {
	pnso_error_t err;
	struct cpdc_init_params init_params;
	struct pnso_checksum_desc pnso_chksum_desc;
	struct service_params svc_params;
	struct service_info svc_info;
	struct pnso_service_status svc_status;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct cpdc_desc *chksum_desc;
	struct cpdc_status_desc *status_desc;
	uint32_t len, count;

	OSAL_LOG_DEBUG("enter ...");

	init_params.cip_version = 0x1234;
	init_params.cip_qdepth = 16;
	init_params.cip_block_size = PNSO_BLOCK_SIZE;

	err = cpdc_init_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	/* use this same setup across UTs */
	count = 1;
	len = 32;

	/* init service params */
	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	pbuf_convert_buffer_list_v2p(src_blist);
	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);
	pbuf_convert_buffer_list_v2p(dst_blist);

	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_MCRC64;
	pnso_chksum_desc.flags = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_chksum_desc = &pnso_chksum_desc;

	/* init service status */
	memset(&svc_status, 0, sizeof(struct pnso_service_status));
	svc_status.err = EINVAL;
	svc_status.svc_type = PNSO_SVC_TYPE_CHKSUM;
	/* TOOD-chksum: per_block or entire buffer init accordingly */
	svc_status.u.dst.sgl = dst_blist;	/* use same buffer for target */

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_CHKSUM;
	svc_info.si_flags = 0xF;
	svc_info.si_ops = chksum_ops;
	svc_info.si_svc_status  = &svc_status;

	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	/* get cp desc and status descriptor */
	chksum_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(chksum_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	/* init status descriptor */
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 0;
	status_desc->csd_partial_data = chksum_desc->cd_status_data;
	status_desc->csd_integrity_data = 0xffff1234eeee5678;

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
	status_desc->csd_partial_data = chksum_desc->cd_status_data;
	svc_status.u.chksum.num_tags = 0;
	err = svc_info.si_ops.write_result(&svc_info);
	EXPECT_EQ(err, PNSO_OK);
	EXPECT_NE(svc_status.u.chksum.num_tags, 0);
	/* ------------------------------------------------------------------ */

	svc_info.si_ops.teardown(&svc_info);
	cpdc_deinit_accelerator();
}

TEST_F(pnso_chksum_test, ut_chksum_teardown) {
	pnso_error_t err;
	struct cpdc_init_params init_params;
	struct pnso_checksum_desc pnso_chksum_desc;
	struct service_params svc_params;
	struct service_info svc_info;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct pnso_flat_buffer *interm_fbuf;
	uint32_t block_size;
	uint32_t len, count;
	uint16_t temp_flags;

	OSAL_LOG_DEBUG("enter ...");

	init_params.cip_version = 0x1234;
	init_params.cip_qdepth = 16;
	init_params.cip_block_size = PNSO_BLOCK_SIZE;

	err = cpdc_init_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	/* use this same setup across UTs */
	count = 1;
	len = 32;
	block_size = init_params.cip_block_size; 

	/* init service params */
	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	pbuf_convert_buffer_list_v2p(src_blist);
	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);
	pbuf_convert_buffer_list_v2p(dst_blist);

	len = PNSO_BUFFER_LEN;
	interm_fbuf = pbuf_aligned_alloc_flat_buffer(PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(interm_fbuf, nullptr);
	pbuf_convert_flat_buffer_v2p(interm_fbuf);

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.sp_interm_fbuf = interm_fbuf;
	svc_params.u.sp_chksum_desc = &pnso_chksum_desc;

	/* init caller's checkksum descriptor */
	pnso_chksum_desc.algo_type = PNSO_CHKSUM_TYPE_MCRC64;
	pnso_chksum_desc.flags = 0;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_CHKSUM;
	svc_info.si_flags = CHAIN_SFLAG_LONE_SERVICE;
	svc_info.si_ops = chksum_ops;
	svc_info.si_interm_fbuf = interm_fbuf;

	/* ------------------------------------------------------------------ */
	OSAL_LOG_INFO("=== verify valid case non-per_block");
	temp_flags = pnso_chksum_desc.flags;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);
	svc_info.si_ops.teardown(&svc_info);
	pnso_chksum_desc.flags = temp_flags;	/* restore original */

	OSAL_LOG_INFO("=== verify valid case per_block");
	svc_info.si_type = PNSO_SVC_TYPE_CHKSUM;
	svc_info.si_flags = CHAIN_SFLAG_LONE_SERVICE;
	svc_info.si_block_size = block_size;
	svc_info.si_ops = chksum_ops;
	svc_info.si_interm_fbuf = interm_fbuf;

	temp_flags = pnso_chksum_desc.flags;
	pnso_chksum_desc.flags = PNSO_CHKSUM_DFLAG_PER_BLOCK;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);
	svc_info.si_ops.teardown(&svc_info);
	pnso_chksum_desc.flags = temp_flags;	/* restore original */

	OSAL_LOG_INFO("=== TODO-chksum_ut: verify mpool count on cp desc/sgl/etc.");
	/* ------------------------------------------------------------------ */

	cpdc_deinit_accelerator();
}

int main(int argc, char **argv) {
    int ret;
    osal_log_init(OSAL_LOG_LEVEL_INFO);
    ::testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    osal_log_deinit();
    return ret;
}
