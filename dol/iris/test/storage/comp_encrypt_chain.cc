// Accelerator compression to XTS-encrypt chaining DOLs.
#include <math.h>
#include "compression.hpp"
#include "compression_test.hpp"
#include "tests.hpp"
#include "utils.hpp"
#include "queues.hpp"
#include "xts.hpp"

#ifdef NDEBUG
#undef NDEBUG
#endif
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <stdio.h>
#include <byteswap.h>
#include "comp_encrypt_chain.hpp"

namespace tests {


/*
 * Constructor
 */
comp_encrypt_chain_t::comp_encrypt_chain_t(comp_encrypt_chain_params_t params) :
    app_max_size(params.app_max_size_),
    app_blk_size(0),
    app_enc_size(params.app_enc_size_),
    caller_comp_pad_buf(nullptr),
    caller_xts_status_vec(nullptr),
    caller_xts_opaque_vec(nullptr),
    caller_xts_opaque_data(0),
    comp_ring(nullptr),
    push_type(ACC_RING_PUSH_INVALID),
    seq_comp_qid(0),
    actual_enc_blks(-1),
    num_enc_blks(0),
    initial_xts_opaque_tag(0),
    last_cp_output_data_len(0),
    last_encrypt_output_data_len(0),
    expected_status(CP_STATUS_SUCCESS),
    force_comp_buf2_bypass(false),
    force_uncomp_encrypt(false),
    destructor_free_buffers(params.destructor_free_buffers_),
    suppress_info_log(params.suppress_info_log_),
    success(false)
{
    uint32_t    max_src_blks;

    uncomp_buf = new dp_mem_t(1, app_max_size,
                              DP_MEM_ALIGN_PAGE, params.uncomp_mem_type_,
                              0, DP_MEM_ALLOC_NO_FILL);

    // Caller can elect to have 2 output buffers, e.g.
    // one in HBM for lower latency P4+ processing, and another in host
    // memory which P4+ will PDMA transfer into for the application.
    comp_buf1 = new dp_mem_t(1, app_max_size,
                             DP_MEM_ALIGN_PAGE, params.comp_mem_type1_,
                             0, DP_MEM_ALLOC_NO_FILL);
    if (params.comp_mem_type2_ != DP_MEM_TYPE_VOID) {
        comp_buf2 = new dp_mem_t(1, comp_buf1->line_size_get(),
                                 DP_MEM_ALIGN_PAGE,
                                 test_mem_type_workaround(params.comp_mem_type2_),
                                 0, DP_MEM_ALLOC_NO_FILL);
    } else {
        comp_buf2 = comp_buf1;
    }
    seq_sgl_pdma = new dp_mem_t(1, sizeof(chain_sgl_pdma_t),
                                DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                sizeof(chain_sgl_pdma_t));

    xts_encrypt_buf = new dp_mem_t(1, app_max_size,
                          DP_MEM_ALIGN_PAGE, params.encrypt_mem_type_,
                          0, DP_MEM_ALLOC_NO_FILL);
    // for comp status, caller can elect to have 2 status buffers, e.g.
    // one in HBM for lower latency P4+ processing, and another in host
    // memory which P4+ will copy into for the application.
    comp_status_buf1 = new dp_mem_t(1, sizeof(cp_status_sha512_t),
                           DP_MEM_ALIGN_SPEC, params.comp_status_mem_type1_,
                           kMinHostMemAllocSize, DP_MEM_ALLOC_NO_FILL);
    if (params.comp_status_mem_type2_ != DP_MEM_TYPE_VOID) {
        comp_status_buf2 = new dp_mem_t(1, sizeof(cp_status_sha512_t),
                               DP_MEM_ALIGN_SPEC, params.comp_status_mem_type2_,
                               kMinHostMemAllocSize, DP_MEM_ALLOC_NO_FILL);
    } else {
        comp_status_buf2 = comp_status_buf1;
    }
    comp_opaque_buf = new dp_mem_t(1, sizeof(uint64_t),
                                   DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                   kMinHostMemAllocSize);

    // XTS AOL must be 512 byte aligned.
    //
    // Note: For certain "source" resources, we allocate twice the number
    // of required entries in order to test P4+ ability to do chaining
    // using the alternate descriptor set when there's a compression error.

    max_enc_blks = COMP_MAX_HASH_BLKS(app_max_size, app_enc_size);
    max_src_blks = max_enc_blks * 2;

    // Workaround model PCIe coherency issue: use HBM based AOLs,
    // opaque buffers, etc.
    xts_src_aol_vec = new dp_mem_t(max_src_blks, sizeof(xts::xts_aol_t),
                                   DP_MEM_ALIGN_SPEC,
                                   test_mem_type_workaround(DP_MEM_TYPE_HOST_MEM), 512);
    xts_desc_vec = new dp_mem_t(max_src_blks, sizeof(xts::xts_desc_t),
                                DP_MEM_ALIGN_SPEC,
                                test_mem_type_workaround(DP_MEM_TYPE_HOST_MEM),
                                sizeof(xts::xts_desc_t));
    xts_dst_aol_vec = new dp_mem_t(max_enc_blks, sizeof(xts::xts_aol_t),
                                   DP_MEM_ALIGN_SPEC,
                                   test_mem_type_workaround(DP_MEM_TYPE_HOST_MEM), 512);
    xts_opaque_vec = new dp_mem_t(max_enc_blks, sizeof(uint64_t),
                                  DP_MEM_ALIGN_SPEC,
                                  test_mem_type_workaround(DP_MEM_TYPE_HOST_MEM),
                                  sizeof(uint64_t));

    // sgl_pad_vec would be used for PDMA of the pad data into comp_buf1
    // unless comp_buf2 is also present, in which case, full PDMA into
    // comp_buf2 would take place using seq_sgl_pdma.
    sgl_pad_vec = new dp_mem_t(max_enc_blks, sizeof(cp_sgl_t),
                               DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                               sizeof(cp_sgl_t));

    // Pre-fill input buffers.
    uint64_t *p64 = (uint64_t *)uncomp_buf->read();
    for (uint64_t i = 0; i < (app_max_size / sizeof(uint64_t)); i++) {
        p64[i] = i;
    }
    uncomp_buf->write_thru();
}


/*
 * Destructor
 */
comp_encrypt_chain_t::~comp_encrypt_chain_t()
{
    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.

    printf("%s success %u destructor_free_buffers %u\n",
           __FUNCTION__, success, destructor_free_buffers);
    if (success && destructor_free_buffers) {
        delete uncomp_buf;
        delete comp_buf1;
        if (comp_buf1 != comp_buf2) {
            delete comp_buf2;
        }
        delete xts_encrypt_buf;
        if (comp_status_buf1 != comp_status_buf2) {
            delete comp_status_buf2;
        }
        delete comp_status_buf1;
        delete comp_opaque_buf;
        delete xts_src_aol_vec;
        delete xts_dst_aol_vec;
        delete xts_desc_vec;
        delete xts_opaque_vec;
        delete sgl_pad_vec;
        delete seq_sgl_pdma;
    }
}


/*
 * Execute any extra pre-push initialization
 */
void
comp_encrypt_chain_t::pre_push(comp_encrypt_chain_pre_push_params_t params)
{
    caller_comp_pad_buf = params.caller_comp_pad_buf_;
    caller_xts_status_vec = params.caller_xts_status_vec_;
    caller_xts_opaque_vec = params.caller_xts_opaque_vec_;
    caller_xts_opaque_data = params.caller_xts_opaque_data_;

    /*
     * XTS opaque data is needed in full_verify() to ensure completion of
     * the decrypt op which is last in the chain, before there's any chance
     * of the caller resubmitting the same chain for another run.
     */
    if (!caller_xts_opaque_vec) {
        caller_xts_opaque_vec = xts_opaque_vec;
        caller_xts_opaque_data = 0xa1a1a1a1a1a1a1a1;

    }
}

/*
 * Initiate the test
 */
int
comp_encrypt_chain_t::push(comp_encrypt_chain_push_params_t params)
{
    chain_params_comp_t chain_params = {0};
    uint32_t            block_no;

    // validate app_blk_size
    if (params.app_blk_size_ > app_max_size) {
        printf("%s app_blk_size %u exceeds app_max_size %u\n", __FUNCTION__,
               params.app_blk_size_, app_max_size);
        assert(params.app_blk_size_ <= app_max_size);
        return -1;
    }

    if (!suppress_info_log) {
        printf("Starting testcase comp_encrypt_chain enc_dec_blk_type %u "
               "app_blk_size %u app_enc_size %u push_type %d "
               "seq_comp_qid %u seq_comp_status_qid %u seq_xts_status_qid %u\n",
               params.enc_dec_blk_type_, params.app_blk_size_, app_enc_size,
               params.push_type_, params.seq_comp_qid_,
               params.seq_comp_status_qid_, params.seq_xts_status_qid_);
    }

    /*
     * Partially overwrite destination buffers to prevent left over
     * data from a previous run
     */
    xts_encrypt_buf->fragment_find(0, sizeof(uint64_t))->fill_thru(0xff);
    comp_buf2->fragment_find(0, sizeof(uint64_t))->fill_thru(0xff);

    enc_dec_blk_type = params.enc_dec_blk_type_;
    app_blk_size = params.app_blk_size_;
    num_enc_blks = COMP_MAX_HASH_BLKS(app_blk_size, app_enc_size);
    actual_enc_blks = -1;
    if (num_enc_blks > max_enc_blks) {
        printf("%s num_enc_blks %u exceeds max_enc_blks %u\n",
               __FUNCTION__, num_enc_blks, max_enc_blks);
        assert(num_enc_blks <= max_enc_blks);
        return -1;
    }

    comp_ring = params.comp_ring_;
    success = false;
    compress_cp_desc_template_fill(cp_desc, uncomp_buf, comp_buf1,
                     comp_status_buf1, nullptr, app_blk_size);

    // XTS chaining will use direct Barco push action from
    // comp status queue handler. Hence, no XTS seq queue needed.
    chain_params.seq_spec.seq_q = params.seq_comp_qid_;
    chain_params.seq_spec.seq_status_q = params.seq_comp_status_qid_;

    // The compression status sequencer will use the AOLs given below
    // only to update the length fields with the correct output data length
    // and pad length.
    xts_src_aol_vec->line_set(0);
    xts_dst_aol_vec->line_set(0);
    chain_params.aol_src_vec_addr = xts_src_aol_vec->pa();
    chain_params.aol_dst_vec_addr = xts_dst_aol_vec->pa();

    // Note: p4+ will modify AOL vectors below based on compression
    // output_data_len and any required padding
    xts_aol_sparse_fill(enc_dec_blk_type, xts_src_aol_vec,
                        comp_buf1, app_enc_size, num_enc_blks);
    xts_aol_sparse_fill(enc_dec_blk_type, xts_dst_aol_vec,
                        xts_encrypt_buf, app_enc_size, num_enc_blks);
    /*
     * Do setup for the alternate descriptor set if applicable
     */
    force_comp_buf2_bypass = params.force_comp_buf2_bypass_;
    force_uncomp_encrypt = params.force_uncomp_encrypt_;
    expected_status = CP_STATUS_SUCCESS;
    if (force_uncomp_encrypt) {

        /*
         * Force compression error to happen by making threshold_len
         * really small. When error happens, we'll end up encrypting
         * the uncompressed data instead.
         */
        cp_desc.threshold_len = sizeof(uint32_t);
        expected_status = CP_STATUS_COMPRESSION_FAILED;
        chain_params.chain_alt_desc_on_error = 1;

        xts_src_aol_vec->line_set(num_enc_blks);
        xts_aol_sparse_fill(enc_dec_blk_type, xts_src_aol_vec,
                            uncomp_buf, app_enc_size, num_enc_blks);
    }

    /*
     * Ensure caller supplied enough status and opaque buffers
     */
    if (enc_dec_blk_type == XTS_ENC_DEC_PER_HASH_BLK) {
        if (caller_xts_status_vec &&
            (caller_xts_status_vec->num_lines_get() < num_enc_blks)) {
            printf("%s number of status buffers %u is less than num_enc_blks %u\n",
                   __FUNCTION__, caller_xts_status_vec->num_lines_get(),
                   num_enc_blks);
            assert(caller_xts_status_vec->num_lines_get() >= num_enc_blks);
            return -1;
        }

        if (caller_xts_opaque_vec &&
            (caller_xts_opaque_vec->num_lines_get() < num_enc_blks)) {
            printf("%s number of opaque buffers %u is less than num_enc_blks %u\n",
                   __FUNCTION__, caller_xts_opaque_vec->num_lines_get(),
                   num_enc_blks);
            assert(caller_xts_opaque_vec->num_lines_get() >= num_enc_blks);
            return -1;
        }

        /*
         * Per-block enryption will require setup of multiple descriptors
         */
        chain_params.desc_vec_push_en = 1;
        chain_params.push_spec.barco_num_descs = num_enc_blks;
        for (block_no = 0; block_no < num_enc_blks; block_no++) {
            encrypt_setup(block_no, block_no, chain_params);

            /*
             * Do setup for the alternate descriptor set if applicable
             */
            if (force_uncomp_encrypt) {
                encrypt_setup(block_no + num_enc_blks, block_no, chain_params);
            }
        }

    } else {
        chain_params.push_spec.barco_num_descs = 1;
        encrypt_setup(0, 0, chain_params);
        if (force_uncomp_encrypt) {
            encrypt_setup(1, 0, chain_params);
        }
    }
    chain_params.num_alt_descs = chain_params.push_spec.barco_num_descs;


    // encryption will use direct Barco push action
    xts_desc_vec->line_set(0);
    chain_params.next_doorbell_en = 1;
    chain_params.next_db_action_barco_push = 1;
    chain_params.push_spec.barco_ring_addr =
                           xts_ctx.acc_ring->ring_base_mem_pa_get();
    chain_params.push_spec.barco_pndx_addr =
                           xts_ctx.acc_ring->cfg_ring_pd_idx_get();
    chain_params.push_spec.barco_pndx_shadow_addr =
                           xts_ctx.acc_ring->shadow_pd_idx_pa_get();
    chain_params.push_spec.barco_desc_addr = xts_desc_vec->pa();
    chain_params.push_spec.barco_desc_size =
                           (uint8_t)log2(xts_ctx.acc_ring->ring_desc_size_get());
    chain_params.push_spec.barco_pndx_size =
                           (uint8_t)log2(xts_ctx.acc_ring->ring_pi_size_get());
    chain_params.push_spec.barco_ring_size =
                           (uint8_t)log2(xts_ctx.acc_ring->ring_size_get());
    comp_status_buf1->fragment_find(0, sizeof(uint64_t))->clear_thru();
    if (comp_status_buf1 != comp_status_buf2) {
        comp_status_buf2->fragment_find(0, sizeof(uint64_t))->clear_thru();
    }
    chain_params.status_addr0 = comp_status_buf1->pa();
    if (comp_status_buf1 != comp_status_buf2) {
        chain_params.status_dma_en = 1;
        chain_params.status_addr1 = comp_status_buf2->pa();
        chain_params.status_len = comp_status_buf2->line_size_get();
    }

    chain_params.pad_buf_addr = caller_comp_pad_buf->pa();
    chain_params.stop_chain_on_error = !force_uncomp_encrypt;
    chain_params.aol_update_en = 1;
    chain_params.padding_en = 1;
    chain_params.pad_boundary_shift =
                 (uint8_t)log2(caller_comp_pad_buf->line_size_get());

    /*
     * Enable sgl_pdma_en, which for comp-encrypt is either full transfer when
     * comp_buf1 != comp_buf2, or pad-only transfer when
     * comp_buf1 == comp_buf2.
     */
    chain_params.sgl_pdma_en = 1;
    if (force_comp_buf2_bypass || (comp_buf1 == comp_buf2)) {
        comp_sgl_packed_fill(sgl_pad_vec, comp_buf1, app_enc_size);
        chain_params.sgl_vec_addr = sgl_pad_vec->pa();
        chain_params.sgl_update_en = 1;
        chain_params.sgl_pdma_pad_only = 1;

    } else {
        chain_sgl_pdma_packed_fill(seq_sgl_pdma, comp_buf2);
        chain_params.comp_buf_addr = comp_buf1->pa();
        chain_params.sgl_vec_addr = seq_sgl_pdma->pa();
    }

    // Enable interrupt in case compression fails
    comp_opaque_buf->clear_thru();
    chain_params.intr_addr = comp_opaque_buf->pa();
    chain_params.intr_data = kCompSeqIntrData;
    chain_params.intr_en = 1;

    if (seq_comp_status_desc_fill(chain_params) != 0) {
        printf("comp_encrypt_chain seq_comp_status_desc_fill failed\n");
        return -1;
    }

    // Chain compression to compression status sequencer
    cp_desc.doorbell_addr = chain_params.seq_spec.ret_doorbell_addr;
    cp_desc.doorbell_data = chain_params.seq_spec.ret_doorbell_data;
    cp_desc.cmd_bits.doorbell_on = 1;

    push_type = params.push_type_;
    seq_comp_qid = params.seq_comp_qid_;
    comp_ring->push((const void *)&cp_desc, params.push_type_, seq_comp_qid);
    return 0;
}


/*
 * Execute any deferred push()
 */
void
comp_encrypt_chain_t::post_push(void)
{
    comp_ring->reentrant_tuple_set(push_type, seq_comp_qid);
    comp_ring->post_push();
}


/*
 * Set up of XTS encryption:
 * src_block_no can be different from dst_block_no when we're setting up
 * the "alternate" source descriptor set.
 */
void
comp_encrypt_chain_t::encrypt_setup(uint32_t src_block_no,
                                    uint32_t dst_block_no,
                                    chain_params_comp_t& chain_params)
{
    xts::xts_cmd_t  cmd;
    xts::xts_desc_t *xts_desc;

    // Use caller's XTS opaque info and status, if any
    if (caller_xts_opaque_vec) {
        caller_xts_opaque_vec->line_set(dst_block_no);
        xts_ctx.xts_db =
            caller_xts_opaque_vec->fragment_find(0, caller_xts_opaque_vec->line_size_get());
        xts_ctx.xts_db_addr = 0;
        xts_ctx.exp_db_data = caller_xts_opaque_data;
        xts_ctx.caller_xts_db_en = true;
    }
    if (caller_xts_status_vec) {
        caller_xts_status_vec->line_set(dst_block_no);
        xts_ctx.status =
            caller_xts_status_vec->fragment_find(0, caller_xts_status_vec->line_size_get());
        xts_ctx.caller_status_en = true;
    }

    // Calling xts_ctx init only to get its xts_db/status initialized
    xts_ctx.init(0, false);
    xts_ctx.suppress_info_log = suppress_info_log;
    xts_ctx.op = xts::AES_ENCR_ONLY;
    xts_ctx.push_type = ACC_RING_PUSH_INVALID;
    if (chain_params.seq_spec.seq_next_q) {
        xts_ctx.push_type = ACC_RING_PUSH_SEQUENCER_BATCH;
        xts_ctx.seq_xts_q = chain_params.seq_spec.seq_next_q;
    }

    // Set up XTS encrypt descriptor
    xts_src_aol_vec->line_set(src_block_no);
    xts_desc_vec->line_set(src_block_no);

    xts_dst_aol_vec->line_set(dst_block_no);

    xts_ctx.cmd_eval_seq_xts(cmd);
    xts_desc = (xts::xts_desc_t *)xts_desc_vec->read();
    xts_ctx.desc_prefill_seq_xts(xts_desc);
    xts_desc->in_aol = xts_src_aol_vec->pa();
    xts_desc->out_aol = xts_dst_aol_vec->pa();
    xts_desc->cmd = cmd;
    xts_desc_vec->write_thru();
    xts_ctx.desc_write_seq_xts(xts_desc);

    // Remember the applicable starting opaque tag for verification later.
    // The value depends on whether we expect to use the alternate descriptor set.
    if ((src_block_no == 0) || (src_block_no == num_enc_blks)) {
        initial_xts_opaque_tag = xts_ctx.last_used_opaque_tag;
    }
}


/*
 * Return actual number of encryption blocks based on the output data length
 * from compression.
 */
int
comp_encrypt_chain_t::actual_enc_blks_get(test_resource_query_method_t query_method)
{
    bool    log_error = (query_method == TEST_RESOURCE_BLOCKING_QUERY);

    if (actual_enc_blks < 0) {

        switch (query_method) {

        case TEST_RESOURCE_BLOCKING_QUERY:

            // Poll for comp status
            if (!comp_status_poll(comp_status_buf2, cp_desc, suppress_info_log)) {
              printf("ERROR: comp_encrypt_chain compression status never came\n");
              return -1;
            }

            /*
             * Fall through!!!
             */

        case TEST_RESOURCE_NON_BLOCKING_QUERY:
        default:

            // Validate comp status
            if (compress_status_verify(comp_status_buf2, comp_buf1, cp_desc,
                                       log_error, expected_status)) {
                if (log_error) {
                    printf("ERROR: comp_encrypt_chain compression status "
                           "verification failed\n");
                }
                return -1;
            }
            last_cp_output_data_len = expected_status == CP_STATUS_SUCCESS ?
                                      comp_status_output_data_len_get(comp_status_buf2) :
                                      app_blk_size;
            actual_enc_blks = enc_dec_blk_type == XTS_ENC_DEC_ENTIRE_APP_BLK ? 1 :
                              (int)COMP_MAX_HASH_BLKS(last_cp_output_data_len,
                                                      app_enc_size);
            if (!suppress_info_log) {
                printf("comp_encrypt_chain: last_cp_output_data_len %u actual_enc_blks %u\n",
                       last_cp_output_data_len, actual_enc_blks);
            }
            if ((actual_enc_blks == 0) || ((uint32_t)actual_enc_blks > num_enc_blks)) {
                printf("comp_encrypt_chain: invalid actual_enc_blks %d in relation to "
                       "num_enc_blks %u\n", actual_enc_blks, num_enc_blks);
                actual_enc_blks = -1;
            }
        }
    }

    return actual_enc_blks;
}


/*
 * Test result verification (fast and non-blocking)
 *
 * Should only be used when caller has another means to ensure that the test
 * has completed. The main purpose of this function to quickly verify operational
 * status, and avoid any lengthy HBM access (such as data comparison) that would
 * slow down test resubmission in the scaled setup.
 */
int
comp_encrypt_chain_t::fast_verify(void)
{
    xts::xts_status_t   *xts_status;
    uint32_t block_no;

    success = false;
    if (actual_enc_blks_get(TEST_RESOURCE_NON_BLOCKING_QUERY) < 0) {
        return -1;
    }

    // Validate XTS status
    for (block_no = 0; block_no < (uint32_t)actual_enc_blks; block_no++) {
        caller_xts_status_vec->line_set(block_no);
        xts_status = (xts::xts_status_t *)caller_xts_status_vec->read_thru();
        if (xts_status->status) {
          printf("ERROR: comp_encrypt_chain XTS error 0x%x\n",
                 xts_status->status);
          return -1;
        }
    }

    if (!suppress_info_log) {
        printf("Testcase comp_encrypt_chain fast_verify passed\n");
    }
    success = true;
    return 0;
}


/*
 * Test result verification (full and possibly blocking)
 *
 * Should only be used in non-scaled setup.
 */
int
comp_encrypt_chain_t::full_verify(void)
{
    xts::xts_aol_t  *xts_aol;
    uint32_t        max_blks;
    uint32_t        block_no;
    uint32_t        exp_encrypt_output_data_len;
    uint64_t        src_len0;
    uint64_t        src_len1;
    uint64_t        src_len2;
    uint64_t        dst_len_total;
    uint32_t        poll_factor = app_blk_size / kCompAppMinSize;

    assert(poll_factor);
    success = false;
    if (actual_enc_blks_get(TEST_RESOURCE_BLOCKING_QUERY) < 0) {
        return -1;
    }

    if (!suppress_info_log) {
        printf("comp_encrypt_chain: last_cp_output_data_len %u\n",
               last_cp_output_data_len);
    }
    exp_encrypt_output_data_len = COMP_MAX_HASH_BLKS(last_cp_output_data_len,
                                                     app_enc_size) * app_enc_size;
    /*
     * Before we start validation of AOLs, ensure that P4+ has had a chance
     * to write them. For that, we wait for completion of at least one
     * encryption block.
     */
    caller_xts_opaque_vec->line_set(0);
    xts_ctx.xts_db =
        caller_xts_opaque_vec->fragment_find(0, caller_xts_opaque_vec->line_size_get());
    if (xts_ctx.verify_doorbell(false, FLAGS_long_poll_interval * poll_factor)) {
        printf("ERROR: comp_encrypt_chain block 0 XTS doorbell engine never came\n");
        return -1;
    }

    /*
     * In XTS_ENC_DEC_ENTIRE_APP_BLK mode, P4+ would have calculated the
     * actual # of blocks required and terminated the last AOL by setting
     * its next pointer to NULL. In the loop below, in order to verify P4+
     * had done the right thing, we iterate to the max # of blocks possible
     * and break when next becomes NULL.
     */
    max_blks = enc_dec_blk_type == XTS_ENC_DEC_ENTIRE_APP_BLK ?
               xts_dst_aol_vec->num_lines_get() : (uint32_t)actual_enc_blks;
    if (!suppress_info_log) {
        xts_aol_trace("comp_encrypt_chain xts_src_aol_vec", xts_src_aol_vec,
                       max_blks, enc_dec_blk_type == XTS_ENC_DEC_ENTIRE_APP_BLK);
        xts_aol_trace("comp_encrypt_chain xts_dst_aol_vec", xts_dst_aol_vec,
                       max_blks, enc_dec_blk_type == XTS_ENC_DEC_ENTIRE_APP_BLK);
    }

    src_len0 = 0;
    src_len1 = 0;
    src_len2 = 0;
    for (block_no = 0; block_no < max_blks; block_no++) {
        if (expected_status == CP_STATUS_SUCCESS) {
            xts_src_aol_vec->line_set(block_no);
        } else {
            xts_src_aol_vec->line_set(block_no + num_enc_blks);
        }
        xts_aol = (xts::xts_aol_t *)xts_src_aol_vec->read_thru();
        src_len0 += xts_aol->l0;
        src_len1 += xts_aol->l1;
        src_len2 += xts_aol->l2;

        /*
         * Re-establish correct pointer to doorbell in XTS context
         * for XTS_ENC_DEC_PER_HASH_BLK mode.
         */
        if (enc_dec_blk_type == XTS_ENC_DEC_PER_HASH_BLK) {
            if (caller_xts_opaque_vec) {
                caller_xts_opaque_vec->line_set(block_no);
                xts_ctx.xts_db =
                    caller_xts_opaque_vec->fragment_find(0, caller_xts_opaque_vec->line_size_get());
            }
        }

        if (xts_ctx.verify_doorbell(false, FLAGS_long_poll_interval * poll_factor)) {
            printf("ERROR: comp_encrypt_chain doorbell from XTS engine never came\n");
            return -1;
        }

        if ((enc_dec_blk_type == XTS_ENC_DEC_ENTIRE_APP_BLK) && !xts_aol->next) {
            break;
        }
    }

    /*
     * Validate total accumulated length
     */
    last_encrypt_output_data_len = src_len0 + src_len1 + src_len2;
    if (last_encrypt_output_data_len != exp_encrypt_output_data_len) {
        printf("ERROR: comp_encrypt_chain last_encrypt_output_data_len %u != "
               "exp_encrypt_output_data_len %u\n", last_encrypt_output_data_len,
               exp_encrypt_output_data_len);
        return -1;
    }

    /*
     * Validate padding length
     */
    if (src_len1 != (exp_encrypt_output_data_len - last_cp_output_data_len)) {
        printf("ERROR: comp_encrypt_chain invalid src_len1 %lu in relation to "
               "exp_encrypt_output_data_len %u and last_cp_output_data_len %u\n",
               src_len1, exp_encrypt_output_data_len, last_cp_output_data_len);
        return -1;
    }

    /*
     * Validate other lengths
     */
    if (src_len2) {
        printf("ERROR: comp_encrypt_chain unexpected src_len2 %lu\n", src_len2);
        return -1;
    }

    /*
     * Validate p4+ writes to xts_dst_aol_vec
     */
    dst_len_total = 0;
    for (block_no = 0; block_no < max_blks; block_no++) {
        xts_dst_aol_vec->line_set(block_no);
        xts_aol = (xts::xts_aol_t *)xts_dst_aol_vec->read_thru();
        dst_len_total += xts_aol->l0 + xts_aol->l1 + xts_aol->l2;
        if ((enc_dec_blk_type == XTS_ENC_DEC_ENTIRE_APP_BLK) && !xts_aol->next) {
            break;
        }
    }

    if (dst_len_total != exp_encrypt_output_data_len) {
        printf("ERROR: comp_encrypt_chain dst_len_total %lu != "
               "exp_encrypt_output_data_len %u\n",
               dst_len_total, exp_encrypt_output_data_len);
        return -1;
    }

    /*
     * Verify individual statuses
     */
    if (fast_verify()) {
        return -1;
    }
    success = false;

    /*
     * Validate PDMA
     */
    if (expected_status == CP_STATUS_SUCCESS) {
        if (force_comp_buf2_bypass || (comp_buf1 == comp_buf2)) {

            /*
             * Validate padding
             */
            if (src_len1 &&
                test_data_verify_and_dump(comp_buf1->read_thru() +
                                          last_cp_output_data_len,
                                          caller_comp_pad_buf->read(),
                                          src_len1)) {
                printf("ERROR: comp_encrypt_chain comp_buf1 pad data "
                       "verification failed\n");
                return -1;
            }

        } else {
            if (test_data_verify_and_dump(comp_buf1->read_thru(),
                                          comp_buf2->read_thru(),
                                          last_cp_output_data_len)) {
                printf("ERROR: comp_encrypt_chain PDMA data "
                       "verification failed\n");
                return -1;
            }
            if (src_len1 &&
                test_data_verify_and_dump(comp_buf2->read() +
                                          last_cp_output_data_len,
                                          caller_comp_pad_buf->read(),
                                          src_len1)) {
                printf("ERROR: comp_encrypt_chain comp_buf2 pad data "
                       "averification failed\n");
                return -1;
            }
        }
    }

    if (!suppress_info_log) {
        printf("Testcase comp_encrypt_chain full_verify passed: "
               "last_encrypt_output_data_len %u\n", last_encrypt_output_data_len);
    }
    success = true;
    return 0;
}

}  // namespace tests
