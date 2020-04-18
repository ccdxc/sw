// Accelerator XTS-decrypt to decompression chaining DOLs.
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
#include "decrypt_decomp_chain.hpp"
#include "comp_encrypt_chain.hpp"

namespace tests {


/*
 * Constructor
 */
decrypt_decomp_chain_t::decrypt_decomp_chain_t(decrypt_decomp_chain_params_t params) :
    comp_encrypt_chain(nullptr),
    app_max_size(params.app_max_size_),
    app_blk_size(0),
    app_enc_size(params.app_enc_size_),
    caller_comp_status_buf(nullptr),
    caller_comp_opaque_buf(nullptr),
    caller_comp_opaque_data(0),
    decomp_ring(nullptr),
    actual_enc_blks(-1),
    num_enc_blks(0),
    last_dc_output_data_len(0),
    last_decrypt_output_data_len(0),
    decomp_bypass(false),
    destructor_free_buffers(params.destructor_free_buffers_),
    suppress_info_log(params.suppress_info_log_),
    success(false)
{
    uncomp_buf = new dp_mem_t(1, app_max_size,
                              DP_MEM_ALIGN_PAGE, params.uncomp_mem_type_,
                              0, DP_MEM_ALLOC_NO_FILL);
    // Caller can elect to have 2 output buffers, e.g.
    // one in HBM for lower latency P4+ processing, and another in host
    // memory which P4+ will PDMA transfer into for the application.
    xts_decrypt_buf1 = new dp_mem_t(1, app_max_size,
                           DP_MEM_ALIGN_PAGE, params.decrypt_mem_type1_,
                           0, DP_MEM_ALLOC_NO_FILL);
    if (params.decrypt_mem_type2_ != DP_MEM_TYPE_VOID) {
        xts_decrypt_buf2 = new dp_mem_t(1, xts_decrypt_buf1->line_size_get(),
                               DP_MEM_ALIGN_PAGE, params.decrypt_mem_type2_,
                               0, DP_MEM_ALLOC_NO_FILL);
    } else {
        xts_decrypt_buf2 = xts_decrypt_buf1;
    }
    seq_sgl_pdma = new dp_mem_t(1, sizeof(chain_sgl_pdma_t),
                                DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                sizeof(chain_sgl_pdma_t));

    // for XTS status, caller can elect to have 2 status vectors, e.g.
    // one in HBM for lower latency P4+ processing, and another in host
    // memory which P4+ will copy into for the application.
    max_enc_blks = COMP_MAX_HASH_BLKS(app_max_size, app_enc_size);
    xts_status_vec1 = new dp_mem_t(max_enc_blks, sizeof(xts::xts_status_t),
                          DP_MEM_ALIGN_SPEC, params.xts_status_mem_type1_,
                          sizeof(xts::xts_status_t), DP_MEM_ALLOC_NO_FILL);
    if (params.xts_status_mem_type2_ != DP_MEM_TYPE_VOID) {
        xts_status_vec2 = new dp_mem_t(max_enc_blks, sizeof(xts::xts_status_t),
                              DP_MEM_ALIGN_SPEC, params.xts_status_mem_type2_,
                              sizeof(xts::xts_status_t), DP_MEM_ALLOC_NO_FILL);
    } else {
        xts_status_vec2 = xts_status_vec1;
    }
    xts_opaque_buf = new dp_mem_t(1, sizeof(uint64_t),
                                  DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                  sizeof(uint64_t));

    // XTS AOL must be 512 byte aligned
    xts_src_aol_vec = new dp_mem_t(max_enc_blks, sizeof(xts::xts_aol_t),
                                   DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM, 512);
    xts_dst_aol_vec = new dp_mem_t(max_enc_blks, sizeof(xts::xts_aol_t),
                                   DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM, 512);
    xts_desc_vec = new dp_mem_t(max_enc_blks, sizeof(xts::xts_desc_t),
                                DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                sizeof(xts::xts_desc_t));
    xts_decomp_cp_desc = new dp_mem_t(1, sizeof(cp_desc_t), DP_MEM_ALIGN_SPEC,
                                      test_mem_type_workaround(DP_MEM_TYPE_HOST_MEM),
                                      sizeof(cp_desc_t));
    decomp_sgl_src_vec = new dp_mem_t(max_enc_blks, sizeof(cp_sgl_t),
                                      DP_MEM_ALIGN_SPEC,
                                      test_mem_type_workaround(DP_MEM_TYPE_HOST_MEM),
                                      sizeof(cp_sgl_t));
}


/*
 * Destructor
 */
decrypt_decomp_chain_t::~decrypt_decomp_chain_t()
{
    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.

    printf("%s success %u destructor_free_buffers %u\n",
           __FUNCTION__, success, destructor_free_buffers);
    if (success && destructor_free_buffers) {
        delete uncomp_buf;
        delete xts_decrypt_buf1;
        if (xts_decrypt_buf1 != xts_decrypt_buf2) {
            delete xts_decrypt_buf2;
        }
        if (xts_status_vec1 != xts_status_vec2) {
            delete xts_status_vec2;
        }
        delete xts_status_vec1;
        delete xts_opaque_buf;
        delete xts_src_aol_vec;
        delete xts_dst_aol_vec;
        delete xts_desc_vec;
        delete xts_decomp_cp_desc;
        delete decomp_sgl_src_vec;
        delete seq_sgl_pdma;
    }
}


/*
 * Execute any extra pre-push initialization
 */
void
decrypt_decomp_chain_t::pre_push(decrypt_decomp_chain_pre_push_params_t params)
{
    caller_comp_status_buf = params.caller_comp_status_buf_;
    caller_comp_opaque_buf = params.caller_comp_opaque_buf_;
    caller_comp_opaque_data = params.caller_comp_opaque_data_;
}

/*
 * Initiate the test
 */
int
decrypt_decomp_chain_t::push(decrypt_decomp_chain_push_params_t params)
{
    chain_params_xts_t  chain_params = {0};
    uint32_t            block_no;

    // validate app_blk_size
    comp_encrypt_chain = params.comp_encrypt_chain_;
    app_blk_size = comp_encrypt_chain->app_blk_size_get();
    app_enc_size = comp_encrypt_chain->app_enc_size_get();
    enc_dec_blk_type = comp_encrypt_chain->enc_dec_blk_type_get();
    actual_enc_blks =
        comp_encrypt_chain->actual_enc_blks_get(TEST_RESOURCE_NON_BLOCKING_QUERY);
    if (app_blk_size > app_max_size) {
        printf("%s app_blk_size %u exceeds app_max_size %u\n", __FUNCTION__,
               app_blk_size, app_max_size);
        assert(app_blk_size <= app_max_size);
        return -1;
    }
    if (actual_enc_blks <= 0) {
        printf("%s invalid actual_enc_blks %d\n", __FUNCTION__, actual_enc_blks);
        assert(actual_enc_blks > 0);
        return -1;
    }

    if (!suppress_info_log) {
        printf("Starting testcase decrypt_decomp_chain enc_dec_blk_type %u "
               "app_blk_size %u app_enc_size %u seq_xts_qid %u seq_xts_status_qid %u "
               "seq_comp_status_qid %u\n", enc_dec_blk_type, app_blk_size,
               app_enc_size, params.seq_xts_qid_,
               params.seq_xts_status_qid_, params.seq_comp_status_qid_);
    }

    /*
     * Execute decomp only if data had been compressed.
     */
    decomp_bypass = comp_encrypt_chain->force_uncomp_encrypt_get();

    /*
     * Partially overwrite destination buffers to prevent left over
     * data from a previous run
     */
    xts_decrypt_buf1->fragment_find(0, sizeof(uint64_t))->fill_thru(0xff);
    xts_decrypt_buf2->fragment_find(0, sizeof(uint64_t))->fill_thru(0xff);
    uncomp_buf->fragment_find(0, sizeof(uint64_t))->fill_thru(0xff);

    memset(&cp_desc, 0, sizeof(cp_desc));
    decomp_ring = params.decomp_ring_;
    success = false;
    if (!decomp_bypass) {
        decompress_cp_desc_template_fill(cp_desc, xts_decrypt_buf1, uncomp_buf,
                           caller_comp_status_buf,
                           comp_encrypt_chain->cp_output_data_len_get(),
                           app_blk_size);
        /*
         * Configure decrypt_decomp_len_update mode
         */
        if (params.decrypt_decomp_len_update_ !=
                   DECRYPT_DECOMP_LEN_UPDATE_NONE) {

            /*
             * Configure the max length possible and let P4+ make the
             * appropriate correction based on the cp_hdr it reads.
             */
            chain_params.comp_len_update_en = 1;
            chain_params.blk_boundary_shift = (uint8_t)log2(app_enc_size);
            cp_desc.datain_len = xts_decrypt_buf1->line_size_get();

            if (params.decrypt_decomp_len_update_ !=
                       DECRYPT_DECOMP_LEN_UPDATE_FLAT) {
                cp_desc.cmd_bits.src_is_list = 1;
                cp_desc.src = decomp_sgl_src_vec->pa();
                chain_params.comp_sgl_src_en = 1;
                chain_params.comp_sgl_src_addr = decomp_sgl_src_vec->pa();

                /*
                 * Validate the mode where the entire SGL describes just
                 * a single buffer.
                 */
                if (params.decrypt_decomp_len_update_ ==
                                   DECRYPT_DECOMP_LEN_UPDATE_SGL_SRC) {
                    comp_sgl_packed_fill(decomp_sgl_src_vec, xts_decrypt_buf1,
                                         xts_decrypt_buf1->line_size_get());

                } else {
                    comp_sgl_packed_fill(decomp_sgl_src_vec, xts_decrypt_buf1,
                                         app_enc_size);
                    chain_params.comp_sgl_src_vec_en = 1;
                }
            }
        }

        caller_comp_status_buf->fragment_find(0, sizeof(uint64_t))->clear_thru();

        // Use caller's Comp opaque info, if any
        if (caller_comp_opaque_buf) {
            cp_desc.cmd_bits.opaque_tag_on = 1;
            cp_desc.opaque_tag_addr = caller_comp_opaque_buf->pa();
            cp_desc.opaque_tag_data = caller_comp_opaque_data;
        }

        /*
         * Decompression will use direct barco push action
         */
        chain_params.next_doorbell_en = 1;
        chain_params.next_db_action_barco_push = 1;
        chain_params.push_spec.barco_ring_addr =
                               decomp_ring->ring_base_mem_pa_get();
        chain_params.push_spec.barco_pndx_addr =
                               decomp_ring->cfg_ring_pd_idx_get();
        chain_params.push_spec.barco_pndx_shadow_addr =
                               decomp_ring->shadow_pd_idx_pa_get();
        chain_params.push_spec.barco_desc_addr = xts_decomp_cp_desc->pa();
        chain_params.push_spec.barco_desc_size =
                               (uint8_t)log2(xts_decomp_cp_desc->line_size_get());
        chain_params.push_spec.barco_pndx_size =
                               (uint8_t)log2(sizeof(uint32_t));
        chain_params.push_spec.barco_ring_size =
                               (uint8_t)log2(decomp_ring->ring_size_get());
        chain_params.push_spec.barco_num_descs = 1;
    }

    if (xts_status_vec1 != xts_status_vec2) {

        // xts_status_vec2 will receive the content of xts_status_vec1
        // so we can freely initialize it with any non-zero (invalid) value.
        xts_status_vec1->line_set(0);
        xts_status_vec2->line_set(0);
        xts_status_vec2->all_lines_fill_thru(0xff);
        chain_params.status_addr0 = xts_status_vec1->pa();
        chain_params.status_addr1 = xts_status_vec2->pa();
        chain_params.status_len =
                     xts_status_vec2->line_size_get() * (uint32_t)actual_enc_blks;
        chain_params.status_dma_en = 1;
    }

    chain_params.seq_spec.seq_q = params.seq_xts_qid_;
    chain_params.seq_spec.seq_status_q = params.seq_xts_status_qid_;

    /*
     * P4+ is capable of initiating chained decomp operation simultaneous
     * with PDMA transfer of the decrypted data. So activate that if the caller
     * had requested it.
     */
    if (xts_decrypt_buf1 != xts_decrypt_buf2) {
        chain_sgl_pdma_packed_fill(seq_sgl_pdma, xts_decrypt_buf2);
        chain_params.sgl_pdma_en = 1;
        chain_params.sgl_pdma_len_from_desc = (params.decrypt_decomp_len_update_ ==
                                               DECRYPT_DECOMP_LEN_UPDATE_NONE);
        chain_params.data_len = comp_encrypt_chain->encrypt_output_data_len_get();
        chain_params.decr_buf_addr = xts_decrypt_buf1->pa();
        chain_params.sgl_pdma_dst_addr = seq_sgl_pdma->pa();
    }

    /*
     * Enable interrupt in case decryption fails;
     * also applicable when decomp is skipped.
     */
    xts_opaque_buf->clear_thru();
    chain_params.intr_addr = xts_opaque_buf->pa();
    chain_params.intr_data = kCompSeqIntrData;
    chain_params.intr_en = 1;
    chain_params.stop_chain_on_error = 1;

    if (xts_ctx.desc_write_seq_xts_status(chain_params)) {
        printf("decrypt_decomp_chain_ desc_write_seq_xts_status failed\n");
        return -1;
    }

    // Set up decryption
    xts_src_aol_vec->line_set(0);
    xts_dst_aol_vec->line_set(0);
    num_enc_blks = comp_encrypt_chain->encrypt_output_data_len_get() /
                   app_enc_size;
    xts_aol_sparse_fill(enc_dec_blk_type, xts_src_aol_vec,
                        comp_encrypt_chain->xts_encrypt_buf_get(),
                        app_enc_size, num_enc_blks);
    xts_aol_sparse_fill(enc_dec_blk_type, xts_dst_aol_vec,
                        xts_decrypt_buf1, app_enc_size, num_enc_blks);
    if (enc_dec_blk_type == XTS_ENC_DEC_PER_HASH_BLK) {

        /*
         * Per-block decryption will require setup of multiple descriptors
         */
        for (block_no = 0; block_no < num_enc_blks; block_no++) {
            decrypt_setup(block_no, chain_params);
        }

    } else {
        decrypt_setup(0, chain_params);
    }

    // Barco push action will operate on the following descriptor
    memcpy(xts_decomp_cp_desc->read(), &cp_desc,
           xts_decomp_cp_desc->line_size_get());
    xts_decomp_cp_desc->write_thru();
    return 0;
}


/*
 * Execute any deferred push()
 */
void
decrypt_decomp_chain_t::post_push(void)
{
    // Initiate the test starting with XTS decryption
    xts_ctx.ring_doorbell();
}


/*
 * Set up of XTS decryption
 */
void
decrypt_decomp_chain_t::decrypt_setup(uint32_t block_no,
                                      chain_params_xts_t& chain_params)
{
    xts::xts_cmd_t  cmd;
    xts::xts_desc_t *xts_desc;

    xts_status_vec1->line_set(block_no);
    xts_ctx.status = xts_status_vec1->fragment_find(0, xts_status_vec1->line_size_get());
    xts_ctx.caller_status_en = true;

    // Calling xts_ctx init only to get its xts_db/status initialized
    xts_ctx.init(0, false);
    xts_ctx.suppress_info_log = suppress_info_log;
    xts_ctx.op = xts::AES_DECR_ONLY;
    xts_ctx.seq_xts_q = chain_params.seq_spec.seq_q;
    xts_ctx.push_type = ACC_RING_PUSH_SEQUENCER_BATCH;

    // Set up XTS encrypt descriptor
    xts_src_aol_vec->line_set(block_no);
    xts_dst_aol_vec->line_set(block_no);
    xts_desc_vec->line_set(block_no);

    xts_ctx.cmd_eval_seq_xts(cmd);
    xts_desc = (xts::xts_desc_t *)xts_desc_vec->read();
    xts_ctx.desc_prefill_seq_xts(xts_desc);
    xts_desc->in_aol = xts_src_aol_vec->pa();
    xts_desc->out_aol = xts_dst_aol_vec->pa();
    xts_desc->cmd = cmd;

    // Chain XTS decrypt of the last block to XTS status sequencer
    if ((enc_dec_blk_type == XTS_ENC_DEC_ENTIRE_APP_BLK) ||
        (block_no == (num_enc_blks - 1))) {

        xts_desc->db_addr = chain_params.seq_spec.ret_doorbell_addr;
        xts_desc->db_data = chain_params.seq_spec.ret_doorbell_data;
    }
    xts_desc_vec->write_thru();
    xts_ctx.desc_write_seq_xts(xts_desc);
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
decrypt_decomp_chain_t::fast_verify(void)
{
    xts::xts_status_t *xts_status;
    cp_desc_t *d;
    uint32_t block_no;

    // Validate XTS status;
    // The actual number of statuses written is based on actual_enc_blks
    // which equals 1 or num_enc_blks.
    for (block_no = 0; block_no < (uint32_t)actual_enc_blks; block_no++) {
        xts_status_vec2->line_set(block_no);
        xts_status = (xts::xts_status_t *)xts_status_vec2->read_thru();
        if (xts_status->status) {
            printf("ERROR: decrypt_decomp_chain XTS error 0x%x\n",
                   xts_status->status);
            return -1;
        }
    }

    // Validate decomp status
    d = (cp_desc_t *)xts_decomp_cp_desc->read_thru();
    if (!decomp_bypass &&
        decompress_status_verify(caller_comp_status_buf, *d, app_blk_size)) {

        printf("ERROR: decrypt_decomp_chain decompression failed: "
               "datain_len %u threshold_len %u\n", d->datain_len, d->threshold_len);
        return -1;
    }

    if (!suppress_info_log) {
        printf("Testcase decrypt_decomp_chain fast_verify passed\n");
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
decrypt_decomp_chain_t::full_verify(void)
{
    cp_desc_t       *d;
    xts::xts_aol_t  *xts_aol;
    xts::xts_status_t   *xts_status = nullptr;
    uint32_t        max_blks;
    uint32_t        block_no;
    uint32_t            poll_factor = app_blk_size / kCompAppMinSize;

    // Don't call xts_ctx.verify_doorbell() as XTS completion would go
    // to XTS status sequencer in the decrypt chaining case.

    // Poll for XTS status
    auto xts_status_poll_func = [this, &xts_status] () -> int {
        uint32_t    block_no;

        // The actual number of statuses written is based on actual_enc_blks
        // which equals 1 or num_enc_blks.
        for (block_no = 0; block_no < (uint32_t)actual_enc_blks; block_no++) {
            xts_status_vec2->line_set(block_no);
            xts_status = (xts::xts_status_t *)xts_status_vec2->read_thru();
            if (xts_status->status) {
                break;
            }
        }

        if (block_no != (uint32_t)actual_enc_blks) {
            return 1;
        }

        /*
         * If second part of chain (decomp) was not executed, ensure XTS
         * interrupt completion.
         */
        if (decomp_bypass) {
            return *((uint32_t *)xts_opaque_buf->read_thru()) ==
                                 kCompSeqIntrData ? 0 : 1;
        }
        return 0;
    };

    assert(poll_factor);
    tests::Poller xts_poll(FLAGS_long_poll_interval * poll_factor);
    success = false;

    if (xts_poll(xts_status_poll_func) != 0) {
        printf("ERROR: decrypt_decomp_chain XTS decrypt error 0x%x\n",
               xts_status->status);
        return -1;
    }

    // Poll for decomp status
    d = (cp_desc_t *)xts_decomp_cp_desc->read_thru();
    if (!decomp_bypass &&
        !comp_status_poll(caller_comp_status_buf, *d, suppress_info_log)) {

        printf("ERROR: decrypt_decomp_chain decompression status never came\n");
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
     * Trace AOL and SGL info
     */
    max_blks = enc_dec_blk_type == XTS_ENC_DEC_ENTIRE_APP_BLK ?
               xts_dst_aol_vec->num_lines_get() : num_enc_blks;
    if (!suppress_info_log) {
        xts_aol_trace("decrypt_decomp_chain xts_src_aol_vec", xts_src_aol_vec,
                       max_blks, enc_dec_blk_type == XTS_ENC_DEC_ENTIRE_APP_BLK);
        xts_aol_trace("decrypt_decomp_chain xts_dst_aol_vec", xts_dst_aol_vec,
                       max_blks, enc_dec_blk_type == XTS_ENC_DEC_ENTIRE_APP_BLK);
        if (d->cmd_bits.src_is_list) {
            comp_sgl_trace("decrypt_decomp_chain decomp_sgl_src_vec",
                           decomp_sgl_src_vec,
                           decomp_sgl_src_vec->num_lines_get(), true);
        }
    }

    /*
     * Validate data
     */
    if (decomp_bypass) {
        if (test_data_verify_and_dump(comp_encrypt_chain->uncomp_data_get(),
                             xts_decrypt_buf2->read_thru(),
                             comp_encrypt_chain->encrypt_output_data_len_get())) {
            printf("ERROR: decrypt_decomp_chain decrypted data "
                   "verification failed\n");
            return -1;
        }

    } else {
        last_dc_output_data_len =
             comp_status_output_data_len_get(caller_comp_status_buf);
        if (!suppress_info_log) {
            printf("decrypt_decomp_chain: last_dc_output_data_len %u\n",
                   last_dc_output_data_len);
        }

        if (test_data_verify_and_dump(comp_encrypt_chain->uncomp_data_get(),
                                      uncomp_buf->read_thru(),
                                      app_blk_size)) {
            printf("ERROR: decrypt_decomp_chain decompressed data "
                   "verification failed\n");
            return -1;
        }

        if (test_data_verify_and_dump(comp_encrypt_chain->comp_data_get(),
                             xts_decrypt_buf2->read_thru(),
                             comp_encrypt_chain->encrypt_output_data_len_get())) {
            printf("ERROR: decrypt_decomp_chain decrypted data "
                   "verification failed\n");
            return -1;
        }
    }

    // Status verification done.
    // Ensure AOL lengths were set correctly.
    last_decrypt_output_data_len = 0;
    for (block_no = 0; block_no < max_blks; block_no++) {
        xts_dst_aol_vec->line_set(block_no);
        xts_aol = (xts::xts_aol_t *)xts_dst_aol_vec->read_thru();
        last_decrypt_output_data_len += xts_aol->l0 + xts_aol->l1 + xts_aol->l2;
        if ((enc_dec_blk_type == XTS_ENC_DEC_ENTIRE_APP_BLK) && !xts_aol->next) {
            break;
        }
    }

    if (last_decrypt_output_data_len !=
        comp_encrypt_chain->encrypt_output_data_len_get()) {
        printf("ERROR: decrypt_decomp_chain last_decrypt_output_data_len %u != "
               "exp_encrypt_output_data_len %u\n", last_decrypt_output_data_len,
               comp_encrypt_chain->encrypt_output_data_len_get());
        return -1;
    }

    if (!suppress_info_log) {
        printf("Testcase decrypt_decomp_chain full_verify passed: "
               "last_decrypt_output_data_len %u\n", last_decrypt_output_data_len);
    }
    success = true;
    return 0;
}

}  // namespace tests
