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
    caller_comp_pad_buf(nullptr),
    caller_xts_status_buf(nullptr),
    caller_xts_opaque_buf(nullptr),
    caller_xts_opaque_data(0),
    comp_queue(nullptr),
    last_cp_output_data_len(0),
    last_encrypt_output_data_len(0),
    destructor_free_buffers(params.destructor_free_buffers_),
    suppress_info_log(params.suppress_info_log_),
    success(false)
{
    uncomp_buf = new dp_mem_t(1, app_max_size,
                              DP_MEM_ALIGN_PAGE, params.uncomp_mem_type_,
                              0, DP_MEM_ALLOC_NO_FILL);

    // size of compressed buffers accounts any for compression failure,
    // i.e., must be as large as uncompressed buffer plus cp_hdr_t
    comp_buf = new dp_mem_t(1, app_max_size + sizeof(cp_hdr_t),
                            DP_MEM_ALIGN_PAGE, params.comp_mem_type_,
                            0, DP_MEM_ALLOC_NO_FILL);
    xts_encrypt_buf = new dp_mem_t(1, app_max_size + sizeof(cp_hdr_t),
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

    // XTS AOL must be 512 byte aligned
    xts_in_aol = new dp_mem_t(1, sizeof(xts::xts_aol_t),
                              DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM, 512);
    xts_out_aol = new dp_mem_t(1, sizeof(xts::xts_aol_t),
                               DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM, 512);
    xts_desc_buf = new dp_mem_t(1, sizeof(xts::xts_desc_t),
                                DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                sizeof(xts::xts_desc_t));
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
        delete comp_buf;
        delete xts_encrypt_buf;
        if (comp_status_buf1 != comp_status_buf2) {
            delete comp_status_buf2;
        }
        delete comp_status_buf1;
        delete comp_opaque_buf;
        delete xts_in_aol;
        delete xts_out_aol;
        delete xts_desc_buf;
    }
}


/*
 * Execute any extra pre-push initialization
 */
void
comp_encrypt_chain_t::pre_push(comp_encrypt_chain_pre_push_params_t params)
{
    caller_comp_pad_buf = params.caller_comp_pad_buf_;
    caller_xts_status_buf = params.caller_xts_status_buf_;
    caller_xts_opaque_buf = params.caller_xts_opaque_buf_;
    caller_xts_opaque_data = params.caller_xts_opaque_data_;
}

/*
 * Initiate the test
 */
int 
comp_encrypt_chain_t::push(comp_encrypt_chain_push_params_t params)
{
    acc_chain_params_t    chain_params = {0};

    // validate app_blk_size
    if (params.app_blk_size_ > app_max_size) {
        printf("%s app_blk_size %u exceeds app_max_size %u\n", __FUNCTION__,
               params.app_blk_size_, app_max_size);
        assert(params.app_blk_size_ <= app_max_size);
        return -1;
    }

    if (!suppress_info_log) {
        printf("Starting testcase comp_encrypt_chain app_blk_size %u push_type %d "
               "seq_comp_qid %u seq_comp_status_qid %u seq_xts_status_qid %u\n",
               params.app_blk_size_, params.push_type_, params.seq_comp_qid_,
               params.seq_comp_status_qid_, params.seq_xts_status_qid_);
    }

    /*
     * Partially overwrite destination buffers to prevent left over
     * data from a previous run
     */
    xts_encrypt_buf->fragment_find(0, sizeof(uint64_t))->fill_thru(0xff);

    app_blk_size = params.app_blk_size_;
    comp_queue = params.comp_queue_;
    success = false;
    compress_cp_desc_template_fill(cp_desc, uncomp_buf, comp_buf,
                     comp_status_buf1, comp_buf, app_blk_size);

    // XTS chaining will use direct Barco push action from
    // comp status queue handler. Hence, no XTS seq queue needed.
    chain_params.desc_format_fn = test_setup_post_comp_seq_status_entry;
    chain_params.seq_q = params.seq_comp_qid_;
    chain_params.seq_status_q = params.seq_comp_status_qid_;

    // Set up encryption
    encrypt_setup(chain_params);

    // encryption will use direct Barco push action
    chain_params.chain_ent.next_doorbell_en = 1;
    chain_params.chain_ent.next_db_action_barco_push = 1;
    chain_params.chain_ent.push_entry.barco_ring_addr = xts_ctx.xts_ring_base_addr;
    chain_params.chain_ent.push_entry.barco_pndx_addr = xts_ctx.xts_ring_pi_addr;
    chain_params.chain_ent.push_entry.barco_pndx_shadow_addr = xts_ctx.xts_ring_pi_shadow_addr->pa();
    chain_params.chain_ent.push_entry.barco_desc_addr = xts_desc_buf->pa();
    chain_params.chain_ent.push_entry.barco_desc_size =
                           (uint8_t)log2(xts_desc_buf->line_size_get());
    chain_params.chain_ent.push_entry.barco_pndx_size =
                           (uint8_t)log2(xts::kXtsPISize);
    chain_params.chain_ent.push_entry.barco_ring_size = (uint8_t)log2(kXtsQueueSize);
    comp_status_buf1->fragment_find(0, sizeof(uint64_t))->clear_thru();
    if (comp_status_buf1 != comp_status_buf2) {
        comp_status_buf2->fragment_find(0, sizeof(uint64_t))->clear_thru();
    }
    chain_params.chain_ent.status_hbm_pa = comp_status_buf1->pa();
    if (comp_status_buf1 != comp_status_buf2) {
        chain_params.chain_ent.status_dma_en = 1;
        chain_params.chain_ent.status_host_pa = comp_status_buf2->pa();
        chain_params.chain_ent.status_len = comp_status_buf2->line_size_get();
    }

    // Note that encryption data transfer is handled by compress_xts_encrypt_setup()
    // above. The compression status sequencer will use the AOLs given below
    // only to update the length fields with the correct output data length
    // and pad length.
    chain_params.chain_ent.barco_aol_in_pa = xts_in_aol->pa();
    chain_params.chain_ent.barco_aol_out_pa = xts_out_aol->pa();
    chain_params.chain_ent.pad_buf_pa = caller_comp_pad_buf->pa();
    chain_params.chain_ent.stop_chain_on_error = 1;
    chain_params.chain_ent.aol_pad_en = 1;
    chain_params.chain_ent.pad_len_shift =
                 (uint8_t)log2(caller_comp_pad_buf->line_size_get());

    // Enable interrupt in case compression fails
    comp_opaque_buf->clear_thru();
    chain_params.chain_ent.intr_pa = comp_opaque_buf->pa();
    chain_params.chain_ent.intr_data = kCompSeqIntrData;
    chain_params.chain_ent.intr_en = 1;

    if (test_setup_seq_acc_chain_entry(chain_params) != 0) {
      printf("test_setup_seq_acc_chain_entry failed\n");
      return -1;
    }

    // Chain compression to compression status sequencer 
    cp_desc.doorbell_addr = chain_params.ret_doorbell_addr;
    cp_desc.doorbell_data = chain_params.ret_doorbell_data;
    cp_desc.cmd_bits.doorbell_on = 1;
    comp_queue->push(cp_desc, params.push_type_, params.seq_comp_qid_);
    return 0;
}


/*
 * Execute any deferred push()
 */
void
comp_encrypt_chain_t::post_push(void)
{
    comp_queue->post_push();
}


/*
 * Set up of XTS encryption
 */
void 
comp_encrypt_chain_t::encrypt_setup(acc_chain_params_t& chain_params)
{
    xts::xts_cmd_t cmd;
    xts::xts_desc_t *xts_desc_addr;
    uint32_t datain_len;

    // Use caller's XTS opaque info and status, if any
    if (caller_xts_opaque_buf) {
        xts_ctx.xts_db = caller_xts_opaque_buf;
        xts_ctx.xts_db_addr = 0;
        xts_ctx.exp_db_data = caller_xts_opaque_data;
        xts_ctx.caller_xts_db_en = true;
    }
    if (caller_xts_status_buf) {
        xts_ctx.status = caller_xts_status_buf;
        xts_ctx.caller_status_en = true;
    }
    xts_ctx.copy_desc = false;
    xts_ctx.ring_db = false;

    // Calling xts_ctx init only to get its xts_db/status initialized
    xts_ctx.init(0, false);
    xts_ctx.op = xts::AES_ENCR_ONLY;
    xts_ctx.use_seq = false;
    if (chain_params.seq_next_q) {
        xts_ctx.use_seq = true;
        xts_ctx.seq_xts_q = chain_params.seq_next_q;
    }
    xts_ctx.copy_desc = false;
    xts_ctx.ring_db = false;

    xts_in_aol->clear();
    xts::xts_aol_t *xts_in = (xts::xts_aol_t *)xts_in_aol->read();

    // Note: p4+ will modify L0 and L1 below based on compression
    // output_data_len and any required padding
    datain_len = cp_desc.datain_len == 0 ?
                 kCompEngineMaxSize :  cp_desc.datain_len;
    xts_in->a0 = comp_buf->pa();
    xts_in->l0 = datain_len;
    xts_in_aol->write_thru();

    xts_out_aol->clear();
    xts::xts_aol_t *xts_out = (xts::xts_aol_t *)xts_out_aol->read();
    xts_out->a0 = xts_encrypt_buf->pa();
    xts_out->l0 = datain_len + sizeof(cp_hdr_t);
    xts_out_aol->write_thru();

    // Set up XTS encrypt descriptor
    xts_ctx.cmd_eval_seq_xts(cmd);
    xts_desc_addr = xts_ctx.desc_prefill_seq_xts(xts_desc_buf);
    xts_desc_addr->in_aol = xts_in_aol->pa();
    xts_desc_addr->out_aol = xts_out_aol->pa();
    xts_desc_addr->cmd = cmd;
    xts_desc_buf->write_thru();
    xts_ctx.desc_write_seq_xts(xts_desc_buf);
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
    // Validate comp status
    success = false;
    if (compress_status_verify(comp_status_buf2, comp_buf, cp_desc)) {
        printf("ERROR: comp_encrypt_chain compression status verification failed\n");
        return -1;
    }

    // Validate XTS status
    uint64_t curr_xts_status = *((uint64_t *)caller_xts_status_buf->read_thru());
    if (curr_xts_status) {
      printf("ERROR: comp_encrypt_chain XTS error 0x%lx\n", curr_xts_status);
      return -1;
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
    uint32_t    poll_factor = app_blk_size / kCompAppMinSize;

    // Poll for comp status
    success = false;
    if (!comp_status_poll(comp_status_buf2, cp_desc, suppress_info_log)) {
      printf("ERROR: comp_encrypt_chain compression status never came\n");
      return -1;
    }

    // Verify XTS engine doorbell
    assert(poll_factor);
    if (xts_ctx.verify_doorbell(false, FLAGS_long_poll_interval * poll_factor)) {
        printf("ERROR: comp_encrypt_chain doorbell from XTS engine never came\n");
        return -1;
    }

    /*
     * Verify individual statuses
     */
    if (fast_verify()) {
        return -1;
    }

    last_cp_output_data_len = comp_status_output_data_len_get(comp_status_buf2);
    if (!suppress_info_log) {
        printf("comp_encrypt_chain: last_cp_output_data_len %u\n",
               last_cp_output_data_len);
    }

    // Status verification done.
    xts::xts_aol_t *xts_out = (xts::xts_aol_t *)xts_out_aol->read_thru();
    last_encrypt_output_data_len = xts_out->l0;

    if (!suppress_info_log) {
        printf("Testcase comp_encrypt_chain full_verify passed: "
               "last_encrypt_output_data_len %u\n", last_encrypt_output_data_len);
    }
    success = true;
    return 0;
}

}  // namespace tests
