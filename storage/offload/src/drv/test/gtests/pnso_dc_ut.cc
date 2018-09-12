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
#include "pnso_chain.h"
#include "pnso_cpdc.h"
#include "pnso_cpdc_cmn.h"

using namespace std;

class pnso_dc_test : public ::testing::Test {
public:

protected:

    pnso_dc_test() {
    }

    virtual ~pnso_dc_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

TEST_F(pnso_dc_test, ut_dc_setup) {
	pnso_error_t err;
	const struct cpdc_init_params init_params = { 0 };
	struct pnso_decompression_desc pnso_dc_desc;
	struct service_params svc_params;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct cpdc_desc *dc_desc;
	struct cpdc_status_desc *status_desc;
	struct service_info svc_info;
	uint32_t len, count;

	uint16_t algo_type;
	uint16_t temp_flags;
	uint32_t temp_len;

	OSAL_LOG_DEBUG("enter ...");

	err = cpdc_init_accelerator(&init_params);
	EXPECT_EQ(err, PNSO_OK);

	/* use this same setup across UTs */
	count = 1;
	len = 32;

	/* init service params */
	src_blist = pbuf_alloc_buffer_list(count, len);
	EXPECT_NE(src_blist, nullptr);
	pbuf_convert_buffer_list_v2p(src_blist);
	// memset((void *) src_blist->buffers[0].buf, 'A', len);
	dst_blist = pbuf_aligned_alloc_buffer_list(count,
			PNSO_MEM_ALIGN_BUF, len);
	EXPECT_NE(dst_blist, nullptr);
	pbuf_convert_buffer_list_v2p(dst_blist);
	// memset((void *) dst_blist->buffers[0].buf, 'B', len);

	pnso_dc_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
	pnso_dc_desc.flags = PNSO_DC_DFLAG_HEADER_PRESENT;
	pnso_dc_desc.hdr_fmt_idx = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_dc_desc = &pnso_dc_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_DECOMPRESS;
	svc_info.si_flags = CHAIN_SFLAG_LONE_SERVICE;
	svc_info.si_ops = dc_ops;

	/* ------------------------------------------------------------------ */
	OSAL_LOG_INFO("=== verify with NULL service info");
	err = svc_info.si_ops.setup(NULL, &svc_params);
	EXPECT_EQ(err, EINVAL);
	OSAL_LOG_INFO("=== verify with NULL service params");
	err = svc_info.si_ops.setup(&svc_info, NULL);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify with invalid src buf list len (=0)");
	temp_len = svc_params.sp_src_blist->buffers[0].len;
	svc_params.sp_src_blist->buffers[0].len = 0;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	svc_params.sp_src_blist->buffers[0].len = temp_len; /* restore original */

	OSAL_LOG_INFO("=== verify with invalid src buf list len (=64K)");
	temp_len = svc_params.sp_src_blist->buffers[0].len;
	svc_params.sp_src_blist->buffers[0].len = MAX_CPDC_SRC_BUF_LEN;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);
	svc_params.sp_src_blist->buffers[0].len = temp_len; /* restore original */

	OSAL_LOG_INFO("=== verify with invalid src buf list len (>64K)");
	temp_len = svc_params.sp_src_blist->buffers[0].len;
	svc_params.sp_src_blist->buffers[0].len = MAX_CPDC_SRC_BUF_LEN + 1;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	svc_params.sp_src_blist->buffers[0].len = temp_len; /* restore original */

	OSAL_LOG_INFO("=== verify invalid algo type");
	algo_type = pnso_dc_desc.algo_type;
	pnso_dc_desc.algo_type = PNSO_COMPRESSION_TYPE_NONE;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	pnso_dc_desc.algo_type = PNSO_COMPRESSION_TYPE_MAX;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	pnso_dc_desc.algo_type = algo_type;	/* restore original */

	OSAL_LOG_INFO("=== verify 'header present' flag");
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);
	dc_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_EQ(dc_desc->u.cd_bits.cc_header_present, 1);
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify no 'header present' flag");
	temp_flags = pnso_dc_desc.flags;
	pnso_dc_desc.flags &= ~PNSO_DC_DFLAG_HEADER_PRESENT;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);
	dc_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_EQ(dc_desc->u.cd_bits.cc_header_present, 0);
	pnso_dc_desc.flags = temp_flags;	/* restore original */
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== verify valid case");
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	dc_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(dc_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_NE(svc_info.si_src_sgl, nullptr);
	EXPECT_NE(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);
	svc_info.si_ops.teardown(&svc_info);
	/* ------------------------------------------------------------------ */

	cpdc_deinit_accelerator();
}

TEST_F(pnso_dc_test, ut_dc_chain) {
	/* TODO-dc_ut: ... */
}

TEST_F(pnso_dc_test, ut_dc_schedule) {
	pnso_error_t err;
	const struct cpdc_init_params init_params = { 0 };
	struct pnso_decompression_desc pnso_dc_desc;
	struct service_params svc_params;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct service_info svc_info;
	uint32_t len, count;

	OSAL_LOG_DEBUG("enter ...");

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

	pnso_dc_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
	pnso_dc_desc.flags = PNSO_DC_DFLAG_HEADER_PRESENT;
	pnso_dc_desc.hdr_fmt_idx = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_dc_desc = &pnso_dc_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_DECOMPRESS;
	svc_info.si_flags = 0xF;
	svc_info.si_ops = dc_ops;

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

TEST_F(pnso_dc_test, ut_dc_poll) {
	/* TODO-dc_ut: ... */
}

TEST_F(pnso_dc_test, ut_dc_read_status) {
	pnso_error_t err;
	const struct cpdc_init_params init_params = { 0 };
	struct pnso_decompression_desc pnso_dc_desc;
	struct service_params svc_params;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct cpdc_desc *dc_desc;
	struct cpdc_status_desc *status_desc;
	struct service_info svc_info;
	uint32_t len, count;

	OSAL_LOG_DEBUG("enter ...");

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

	pnso_dc_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
	pnso_dc_desc.flags = PNSO_DC_DFLAG_HEADER_PRESENT;
	pnso_dc_desc.hdr_fmt_idx = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_dc_desc = &pnso_dc_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_DECOMPRESS;
	svc_info.si_flags = 0xF;
	svc_info.si_ops = dc_ops;

	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	/* get dc desc and status descriptor */
	dc_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(dc_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	/* init status descriptor */
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 0;
	status_desc->csd_output_data_len = dc_desc->cd_datain_len;
	status_desc->csd_partial_data = dc_desc->cd_status_data;

	/* ------------------------------------------------------------------ */
	OSAL_LOG_INFO("=== verify valid bit not set");
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 0;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify hw error set");
	status_desc->csd_err = 0x3;	/* fake the error */
	status_desc->csd_valid = 1;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, 0x3);

	OSAL_LOG_INFO("=== verify status and partial data mismatch");
	status_desc->csd_err = 0x4; 	/* fake the error */
	status_desc->csd_valid = 1;
	status_desc->csd_partial_data = dc_desc->cd_status_data + 1;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, 0x4);

	OSAL_LOG_INFO("=== verify integrity data not set");
	status_desc->csd_err = 0x5; 	/* fake the error */
	status_desc->csd_valid = 1;
	status_desc->csd_partial_data = dc_desc->cd_status_data;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, 0x5);

	OSAL_LOG_INFO("=== verify valid case");
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 1;
	status_desc->csd_output_data_len = dc_desc->cd_datain_len;
	status_desc->csd_partial_data = dc_desc->cd_status_data;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, PNSO_OK);
	/* ------------------------------------------------------------------ */

	svc_info.si_ops.teardown(&svc_info);
	cpdc_deinit_accelerator();
}

TEST_F(pnso_dc_test, ut_dc_write_result) {
	pnso_error_t err;
	const struct cpdc_init_params init_params = { 0 };
	struct pnso_decompression_desc pnso_dc_desc;
	struct service_params svc_params;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct cpdc_desc *dc_desc;
	struct cpdc_status_desc *status_desc;
	struct service_info svc_info;
	struct pnso_service_status svc_status;
	uint32_t len, count;

	OSAL_LOG_DEBUG("enter ...");

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

	pnso_dc_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
	pnso_dc_desc.flags = PNSO_DC_DFLAG_HEADER_PRESENT;
	pnso_dc_desc.hdr_fmt_idx = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_dc_desc = &pnso_dc_desc;

	/* init service status */
	memset(&svc_status, 0, sizeof(struct pnso_service_status));
	svc_status.err = EINVAL;
	svc_status.svc_type = PNSO_SVC_TYPE_DECOMPRESS;
	svc_status.u.dst.data_len = 0;
	svc_status.u.dst.sgl = dst_blist;	/* use same buffer for target */

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_DECOMPRESS;
	svc_info.si_flags = 0xF;
	svc_info.si_ops = dc_ops;
	svc_info.si_svc_status  = &svc_status;

	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	/* get dc desc and status descriptor */
	dc_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(dc_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	/* init status descriptor */
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 0;
	status_desc->csd_output_data_len = dc_desc->cd_datain_len;
	status_desc->csd_partial_data = dc_desc->cd_status_data;

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
	status_desc->csd_output_data_len = dc_desc->cd_datain_len;
	status_desc->csd_partial_data = dc_desc->cd_status_data;
	svc_status.u.dst.data_len = 0;
	err = svc_info.si_ops.write_result(&svc_info);
	EXPECT_EQ(err, PNSO_OK);
	EXPECT_NE(svc_status.u.dst.data_len, 0);
	/* ------------------------------------------------------------------ */

	svc_info.si_ops.teardown(&svc_info);
	cpdc_deinit_accelerator();
}

TEST_F(pnso_dc_test, ut_dc_teardown) {
	pnso_error_t err;
	const struct cpdc_init_params init_params = { 0 };
	struct pnso_decompression_desc pnso_dc_desc;
	struct service_params svc_params;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct service_info svc_info;
	uint32_t len, count;

	OSAL_LOG_DEBUG("enter ...");

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

	pnso_dc_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
	pnso_dc_desc.flags = PNSO_DC_DFLAG_HEADER_PRESENT;
	pnso_dc_desc.hdr_fmt_idx = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_dc_desc = &pnso_dc_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_DECOMPRESS;
	svc_info.si_flags = CHAIN_SFLAG_LONE_SERVICE;
	svc_info.si_ops = dc_ops;

	/* ------------------------------------------------------------------ */
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== TODO-dc_ut: verify mpool count on dc desc/sgl/etc.");
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
