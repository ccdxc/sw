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

class pnso_cp_test : public ::testing::Test {
public:

protected:

    pnso_cp_test() {
    }

    virtual ~pnso_cp_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

TEST_F(pnso_cp_test, ut_cp_setup) {
	pnso_error_t err;
	const struct cpdc_init_params init_params = { 0 };
	struct pnso_compression_desc pnso_cp_desc;
	struct service_params svc_params;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct cpdc_desc *cp_desc;
	struct cpdc_status_desc *status_desc;
	struct service_info svc_info;
	uint32_t len, count;

	uint16_t algo_type;
	uint16_t temp_flags;
	uint32_t temp_len;

	OSAL_LOG_INFO("enter ...");

	err = cpdc_start_accelerator(&init_params);
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

	pnso_cp_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
	pnso_cp_desc.flags = PNSO_CP_DFLAG_INSERT_HEADER;
	pnso_cp_desc.threshold_len = len -
		sizeof(struct pnso_compression_header);
	pnso_cp_desc.hdr_fmt_idx = 0;
	pnso_cp_desc.hdr_algo = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_cp_desc = &pnso_cp_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_COMPRESS;
	svc_info.si_flags = CHAIN_SFLAG_LONE_SERVICE;
	svc_info.si_ops = cp_ops;

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
	algo_type = pnso_cp_desc.algo_type;
	pnso_cp_desc.algo_type = PNSO_COMPRESSION_TYPE_NONE;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	pnso_cp_desc.algo_type = PNSO_COMPRESSION_TYPE_MAX;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);
	pnso_cp_desc.algo_type = algo_type;	/* restore to original */

	OSAL_LOG_INFO("=== verify invalid threshold len");
	temp_len = pnso_cp_desc.threshold_len;
	pnso_cp_desc.threshold_len = MAX_CP_THRESHOLD_LEN - 1;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify valid threshold len");
	pnso_cp_desc.threshold_len = MAX_CP_THRESHOLD_LEN -
		sizeof(struct pnso_compression_header);
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);
	pnso_cp_desc.threshold_len = temp_len;	/* restore to original */

	OSAL_LOG_INFO("=== verify no 'insert header' flag");
	temp_flags = pnso_cp_desc.flags;
	pnso_cp_desc.flags &= ~PNSO_CP_DFLAG_INSERT_HEADER;
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);
	cp_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_EQ(cp_desc->u.cd_bits.cc_insert_header, 0);
	pnso_cp_desc.flags = temp_flags;	/* restore to original */

	OSAL_LOG_INFO("=== TODO-cp_ut: verify invalid flags, header algo, fmt");
	OSAL_LOG_INFO("=== TODO-cp_ut: verify mpool count on error recovery");

	OSAL_LOG_INFO("=== verify valid case");
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	cp_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(cp_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	EXPECT_NE(svc_info.si_src_sgl, nullptr);
	EXPECT_NE(svc_info.si_dst_sgl, nullptr);
	EXPECT_EQ(svc_info.si_svc_status, nullptr);
	/* ------------------------------------------------------------------ */

	svc_info.si_ops.teardown(&svc_info);
	cpdc_stop_accelerator();
}

TEST_F(pnso_cp_test, ut_cp_chain) {
	/* TODO-cp_ut: ... */
}

TEST_F(pnso_cp_test, ut_cp_schedule) {
	pnso_error_t err;
	const struct cpdc_init_params init_params = { 0 };
	struct pnso_compression_desc pnso_cp_desc;
	struct service_params svc_params;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct service_info svc_info;
	uint32_t len, count;

	OSAL_LOG_INFO("enter ...");

	err = cpdc_start_accelerator(&init_params);
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

	pnso_cp_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
	pnso_cp_desc.flags = PNSO_CP_DFLAG_INSERT_HEADER;
	pnso_cp_desc.threshold_len = len -
		sizeof(struct pnso_compression_header);
	pnso_cp_desc.hdr_fmt_idx = 0;
	pnso_cp_desc.hdr_algo = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_cp_desc = &pnso_cp_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_COMPRESS;
	svc_info.si_flags = 0xF;
	svc_info.si_ops = cp_ops;

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

TEST_F(pnso_cp_test, ut_cp_poll) {
	/* TODO-cp_ut: ... */
}

TEST_F(pnso_cp_test, ut_cp_read_status) {
	pnso_error_t err;
	const struct cpdc_init_params init_params = { 0 };
	struct pnso_compression_desc pnso_cp_desc;
	struct service_params svc_params;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct cpdc_desc *cp_desc;
	struct cpdc_status_desc *status_desc;
	struct service_info svc_info;
	uint32_t len, count;

	OSAL_LOG_INFO("enter ...");

	err = cpdc_start_accelerator(&init_params);
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

	pnso_cp_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
	pnso_cp_desc.flags = PNSO_CP_DFLAG_INSERT_HEADER;
	pnso_cp_desc.threshold_len = len -
		sizeof(struct pnso_compression_header);
	pnso_cp_desc.hdr_fmt_idx = 0;
	pnso_cp_desc.hdr_algo = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_cp_desc = &pnso_cp_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_COMPRESS;
	svc_info.si_flags = 0xF;
	svc_info.si_ops = cp_ops;

	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	/* get cp desc and status descriptor */
	cp_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(cp_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	/* init status descriptor */
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 0;
	status_desc->csd_output_data_len =
		cp_desc->cd_datain_len - cp_desc->cd_threshold_len;
	status_desc->csd_partial_data = cp_desc->cd_status_data;
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
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify status and partial data mismatch");
	status_desc->csd_err = 0x1; 	/* fake the error */
	status_desc->csd_valid = 1;
	status_desc->csd_partial_data = cp_desc->cd_status_data + 1;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify integrity data not set");
	status_desc->csd_err = 0x1; 	/* fake the error */
	status_desc->csd_valid = 1;
	status_desc->csd_partial_data = cp_desc->cd_status_data;
	// status_desc->csd_integrity_data = 0;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify valid case");
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 1;
	status_desc->csd_output_data_len =
		cp_desc->cd_datain_len - cp_desc->cd_threshold_len;
	status_desc->csd_partial_data = cp_desc->cd_status_data;
	// status_desc->csd_integrity_data = 0xffff1234eeee5678;
	err = svc_info.si_ops.read_status(&svc_info);
	EXPECT_EQ(err, PNSO_OK);
	/* ------------------------------------------------------------------ */

	svc_info.si_ops.teardown(&svc_info);
	cpdc_stop_accelerator();
}

TEST_F(pnso_cp_test, ut_cp_write_result) {
	pnso_error_t err;
	const struct cpdc_init_params init_params = { 0 };
	struct pnso_compression_desc pnso_cp_desc;
	struct service_params svc_params;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct cpdc_desc *cp_desc;
	struct cpdc_status_desc *status_desc;
	struct service_info svc_info;
	struct pnso_service_status svc_status;
	uint32_t len, count;

	OSAL_LOG_INFO("enter ...");

	err = cpdc_start_accelerator(&init_params);
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

	pnso_cp_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
	pnso_cp_desc.flags = PNSO_CP_DFLAG_INSERT_HEADER;
	pnso_cp_desc.threshold_len = len - 
		sizeof(struct pnso_compression_header);
	pnso_cp_desc.hdr_fmt_idx = 0;
	pnso_cp_desc.hdr_algo = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_cp_desc = &pnso_cp_desc;

	/* init service status */
	memset(&svc_status, 0, sizeof(struct pnso_service_status));
	svc_status.err = EINVAL;
	svc_status.svc_type = PNSO_SVC_TYPE_COMPRESS;
	svc_status.u.dst.data_len = 0;
	svc_status.u.dst.sgl = dst_blist;	/* use same buffer for target */

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_COMPRESS;
	svc_info.si_flags = 0xF;
	svc_info.si_ops = cp_ops;
	svc_info.si_svc_status  = &svc_status;

	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);

	/* get cp desc and status descriptor */
	cp_desc = (struct cpdc_desc *) svc_info.si_desc;
	EXPECT_NE(cp_desc, nullptr);
	status_desc = (struct cpdc_status_desc *) svc_info.si_status_desc;
	EXPECT_NE(status_desc, nullptr);

	/* init status descriptor */
	status_desc->csd_err = PNSO_OK;
	status_desc->csd_valid = 0;
	status_desc->csd_output_data_len =
		cp_desc->cd_datain_len - cp_desc->cd_threshold_len;
	status_desc->csd_partial_data = cp_desc->cd_status_data;
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
	status_desc->csd_output_data_len =
		cp_desc->cd_datain_len - cp_desc->cd_threshold_len;
	status_desc->csd_partial_data = cp_desc->cd_status_data;
	// status_desc->csd_integrity_data = 0xffff1234eeee5678;
	svc_status.u.dst.data_len = 0;
	err = svc_info.si_ops.write_result(&svc_info);
	EXPECT_EQ(err, PNSO_OK);
	EXPECT_NE(svc_status.u.dst.data_len, 0);
	/* ------------------------------------------------------------------ */

	svc_info.si_ops.teardown(&svc_info);
	cpdc_stop_accelerator();
}

TEST_F(pnso_cp_test, ut_cp_teardown) {
	pnso_error_t err;
	const struct cpdc_init_params init_params = { 0 };
	struct pnso_compression_desc pnso_cp_desc;
	struct service_params svc_params;
	struct pnso_buffer_list *src_blist;
	struct pnso_buffer_list *dst_blist;
	struct service_info svc_info;
	uint32_t len, count;

	OSAL_LOG_INFO("enter ...");

	err = cpdc_start_accelerator(&init_params);
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

	pnso_cp_desc.algo_type = PNSO_COMPRESSION_TYPE_LZRW1A;
	pnso_cp_desc.flags = PNSO_CP_DFLAG_INSERT_HEADER;
	pnso_cp_desc.threshold_len = len -
		sizeof(struct pnso_compression_header);
	pnso_cp_desc.hdr_fmt_idx = 0;
	pnso_cp_desc.hdr_algo = 0;

	svc_params.sp_src_blist = src_blist;
	svc_params.sp_dst_blist = dst_blist;
	svc_params.u.sp_cp_desc = &pnso_cp_desc;

	/* init service info */
	memset(&svc_info, 0, sizeof(struct service_info));
	svc_info.si_type = PNSO_SVC_TYPE_COMPRESS;
	svc_info.si_flags = CHAIN_SFLAG_LONE_SERVICE;
	svc_info.si_ops = cp_ops;

	/* ------------------------------------------------------------------ */
	err = svc_info.si_ops.setup(&svc_info, &svc_params);
	EXPECT_EQ(err, PNSO_OK);
	svc_info.si_ops.teardown(&svc_info);

	OSAL_LOG_INFO("=== TODO-cp_ut: verify mpool count on cp desc/sgl/etc.");
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
