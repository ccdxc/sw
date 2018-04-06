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
    caller_comp_status_buf(nullptr),
    caller_comp_opaque_buf(nullptr),
    caller_comp_opaque_data(0),
    decomp_queue(nullptr),
    last_dc_output_data_len(0),
    last_decrypt_output_data_len(0),
    destructor_free_buffers(params.destructor_free_buffers_),
    suppress_info_log(params.suppress_info_log_),
    success(false)
{
    uncomp_buf = new dp_mem_t(1, app_max_size,
                              DP_MEM_ALIGN_PAGE, params.uncomp_mem_type_);

    // size of compressed buffers accounts any for compression failure,
    // i.e., must be as large as uncompressed buffer plus cp_hdr_t
    xts_decrypt_buf = new dp_mem_t(1, app_max_size + sizeof(cp_hdr_t),
                          DP_MEM_ALIGN_PAGE, params.decrypt_mem_type_);
    // for XTS status, caller can elect to have 2 status buffers, e.g.
    // one in HBM for lower latency P4+ processing, and another in host
    // memory which P4+ will copy into for the application.
    xts_status_buf1 = new dp_mem_t(1, sizeof(uint32_t),
                          DP_MEM_ALIGN_NONE, params.xts_status_mem_type1_);
    if (params.xts_status_mem_type2_ != DP_MEM_TYPE_VOID) {
        xts_status_buf2 = new dp_mem_t(1, sizeof(uint32_t),
                              DP_MEM_ALIGN_NONE, params.xts_status_mem_type2_);
    } else {
        xts_status_buf2 = xts_status_buf1;
    }
    xts_opaque_buf = new dp_mem_t(1, sizeof(uint64_t),
                                  DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);

    // XTS AOL must be 512 byte aligned
    xts_in_aol = new dp_mem_t(1, sizeof(xts::xts_aol_t),
                              DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM, 512);
    xts_out_aol = new dp_mem_t(1, sizeof(xts::xts_aol_t),
                               DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM, 512);
    xts_desc_buf = new dp_mem_t(1, sizeof(xts::xts_desc_t),
                                DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM,
                                sizeof(xts::xts_desc_t));
    xts_decomp_cp_desc = new dp_mem_t(1, sizeof(cp_desc_t), DP_MEM_ALIGN_SPEC,
                                      DP_MEM_TYPE_HBM, sizeof(cp_desc_t));
}


/*
 * Destructor
 */
decrypt_decomp_chain_t::~decrypt_decomp_chain_t()
{
    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.
    if (success && destructor_free_buffers) {
        delete uncomp_buf;
        delete xts_decrypt_buf;
        if (xts_status_buf1 != xts_status_buf2) {
            delete xts_status_buf2;
        }
        delete xts_status_buf1;
        delete xts_opaque_buf;
        delete xts_in_aol;
        delete xts_out_aol;
        delete xts_desc_buf;
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
    acc_chain_params_t  chain_params = {0};

    // validate app_blk_size
    app_blk_size = params.comp_encrypt_chain_->app_blk_size_get();
    if (app_blk_size > app_max_size) {
        printf("%s app_blk_size %u exceeds app_max_size %u\n", __FUNCTION__,
               app_blk_size, app_max_size);
        assert(app_blk_size <= app_max_size);
        return -1;
    }

    if (!suppress_info_log) {
        printf("Starting testcase decrypt_decomp_chain app_blk_size %u "
               "seq_xts_qid %u seq_xts_status_qid %u seq_comp_status_qid %u\n",
               app_blk_size, params.seq_xts_qid_,
               params.seq_xts_status_qid_, params.seq_comp_status_qid_);
    }

    /*
     * Partially overwrite destination buffers to prevent left over
     * data from a previous run
     */
    xts_decrypt_buf->fragment_find(0, 64)->fill_thru(0xff);
    uncomp_buf->fragment_find(0, 64)->fill_thru(0xff);

    comp_encrypt_chain = params.comp_encrypt_chain_;
    decomp_queue = params.decomp_queue_;
    success = false;
    decompress_cp_desc_template_fill(cp_desc, xts_decrypt_buf, uncomp_buf,
                                     caller_comp_status_buf,
                                     comp_encrypt_chain->cp_output_data_len_get(),
                                     app_blk_size);
    // Use caller's Comp opaque info, if any
    if (caller_comp_opaque_buf) {
        cp_desc.cmd_bits.opaque_tag_on = 1;
        cp_desc.opaque_tag_addr = caller_comp_opaque_buf->pa();
        cp_desc.opaque_tag_data = caller_comp_opaque_data;
    }

    chain_params.desc_format_fn = test_setup_post_xts_seq_status_entry;
    chain_params.seq_q = params.seq_xts_qid_;
    chain_params.seq_status_q = params.seq_xts_status_qid_;

    // Decompression will use direct barco push action
    chain_params.chain_ent.next_doorbell_en = 1;
    chain_params.chain_ent.next_db_action_barco_push = 1;
    chain_params.chain_ent.push_entry.barco_ring_addr = decomp_queue->q_base_mem_pa_get();
    chain_params.chain_ent.push_entry.barco_pndx_addr = decomp_queue->cfg_q_pd_idx_get();
    chain_params.chain_ent.push_entry.barco_desc_addr = xts_decomp_cp_desc->pa();
    chain_params.chain_ent.push_entry.barco_desc_size = 
                           (uint8_t)log2(xts_decomp_cp_desc->line_size_get());
    chain_params.chain_ent.push_entry.barco_pndx_size = 
                           (uint8_t)log2(sizeof(uint32_t));
    if (xts_status_buf1 != xts_status_buf2) {
        xts_status_buf2->clear_thru();
        chain_params.chain_ent.status_hbm_pa = xts_status_buf1->pa();
        chain_params.chain_ent.status_host_pa = xts_status_buf2->pa();
        chain_params.chain_ent.status_len = xts_status_buf2->line_size_get();
        chain_params.chain_ent.status_dma_en = 1;
    }

    // Enable interrupt in case decryption fails
    xts_opaque_buf->clear_thru();
    chain_params.chain_ent.intr_pa = xts_opaque_buf->pa();
    chain_params.chain_ent.intr_data = kCompSeqIntrData;
    chain_params.chain_ent.intr_en = 1;
    chain_params.chain_ent.stop_chain_on_error = 1;

    if (test_setup_seq_acc_chain_entry(chain_params) != 0) {
      printf("test_setup_seq_acc_chain_entry failed\n");
      return -1;
    }

    // Set up decryption
    decrypt_setup(chain_params);

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
 * Set up of XTS encryption
 */
void 
decrypt_decomp_chain_t::decrypt_setup(acc_chain_params_t& chain_params)
{
    xts::xts_cmd_t  cmd;
    xts::xts_desc_t *xts_desc_addr;

    // Calling xts_ctx init only to get its xts_db initialized
    xts_ctx.init(0, false);
    xts_ctx.op = xts::AES_DECR_ONLY;
    xts_ctx.use_seq = true;
    xts_ctx.seq_xts_q = chain_params.seq_q;
    xts_ctx.seq_xts_status_q = chain_params.seq_status_q;
    xts_ctx.copy_desc = false;
    xts_ctx.ring_db = false;

    xts_in_aol->clear();
    xts::xts_aol_t *xts_in = (xts::xts_aol_t *)xts_in_aol->read();

    xts_in->a0 = comp_encrypt_chain->xts_encrypt_buf_get()->pa();
    xts_in->l0 = comp_encrypt_chain->encrypt_output_data_len_get();
    xts_in_aol->write_thru();

    xts_out_aol->clear();
    xts::xts_aol_t *xts_out = (xts::xts_aol_t *)xts_out_aol->read();
    xts_out->a0 = xts_decrypt_buf->pa();
    xts_out->l0 = xts_in->l0;
    xts_out_aol->write_thru();

    // Set up XTS decrypt descriptor
    xts_status_buf1->clear_thru();
    xts_ctx.cmd_eval_seq_xts(cmd);
    xts_desc_addr = xts_ctx.desc_prefill_seq_xts(xts_desc_buf);
    xts_desc_addr->in_aol = xts_in_aol->pa();
    xts_desc_addr->out_aol = xts_out_aol->pa();
    xts_desc_addr->cmd = cmd;
    xts_desc_addr->status = xts_status_buf1->pa();

    // Chain XTS decrypt to XTS status sequencer 
    queues::get_capri_doorbell(queues::get_seq_lif(), SQ_TYPE,
                               xts_ctx.seq_xts_status_q, 0,
                               chain_params.ret_seq_status_index, 
                               &xts_desc_addr->db_addr, &xts_desc_addr->db_data);
    xts_desc_buf->write_thru();
    xts_ctx.desc_write_seq_xts(xts_desc_buf);
}

/*
 * Test result verification
 */
int 
decrypt_decomp_chain_t::verify(void)
{
    // Don't call xts_ctx.verify_doorbell() as XTS completion would go
    // to XTS status sequenceer in the decrypt chaining case.

    // Poll for XTS status
    auto xts_status_poll_func = [this] () -> int {
      uint32_t curr_xts_status = *((uint32_t *)xts_status_buf2->read_thru());
      if (!curr_xts_status)
        return 0;
      return 1;
    };

    tests::Poller xts_poll;
    if (xts_poll(xts_status_poll_func) != 0) {
      uint32_t curr_xts_status = *((uint32_t *)xts_status_buf2->read());
      printf("ERROR: XTS decrypt error 0x%x\n", curr_xts_status);
      return -1;
    }

    // Poll for decomp status
    if (!comp_status_poll(caller_comp_status_buf)) {
      printf("ERROR: decompression status never came\n");
      return -1;
    }

    // Validate decomp status
    if (decompress_status_verify(caller_comp_status_buf, cp_desc, app_blk_size)) {
      printf("ERROR: decompression failed\n");
      return -1;
    }
    last_dc_output_data_len = comp_status_output_data_len_get(caller_comp_status_buf);

    // Validate data
    if (test_data_verify_and_dump(comp_encrypt_chain->uncomp_data_get(),
                                  uncomp_buf->read_thru(),
                                  app_blk_size)) {
        printf("ERROR: data verification failed\n");
        return -1;
    }

    // Status verification done.
    xts::xts_aol_t *xts_out = (xts::xts_aol_t *)xts_out_aol->read_thru();
    last_decrypt_output_data_len = xts_out->l0;

    if (!suppress_info_log) {
        printf("Testcase decrypt_decomp_chain passed\n");
    }
    success = true;
    return 0;
}

}  // namespace tests
