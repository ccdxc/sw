// Accelerator XTS-encrypt DOLs.
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
#include "encrypt_only.hpp"

namespace tests {


/*
 * Constructor
 */
encrypt_only_t::encrypt_only_t(encrypt_only_params_t params) :
    app_max_size(params.app_max_size_),
    app_blk_size(0),
    caller_xts_status_buf(nullptr),
    caller_xts_opaque_buf(nullptr),
    caller_xts_opaque_data(0),
    destructor_free_buffers(params.destructor_free_buffers_),
    suppress_info_log(params.suppress_info_log_),
    success(false)
{
    uint32_t    num_dwords;

    unencrypt_buf = new dp_mem_t(1, app_max_size,
                                 DP_MEM_ALIGN_PAGE, params.unencrypt_mem_type_,
                                 0, DP_MEM_ALLOC_NO_FILL);
    xts_encrypt_buf = new dp_mem_t(1, app_max_size,
                          DP_MEM_ALIGN_PAGE, params.encrypt_mem_type_,
                          0, DP_MEM_ALLOC_NO_FILL);
    xts_status_buf = new dp_mem_t(1, sizeof(xts::xts_status_t),
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
    uint64_t *p64 = (uint64_t *)unencrypt_buf->read();
    num_dwords = app_max_size / sizeof(uint64_t);
    for (uint32_t i = 0; i < num_dwords; i++) {
        p64[i] = num_dwords - i;
    }
    unencrypt_buf->write_thru();
}


/*
 * Destructor
 */
encrypt_only_t::~encrypt_only_t()
{
    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.

    printf("%s success %u destructor_free_buffers %u\n",
           __FUNCTION__, success, destructor_free_buffers);
    if (success && destructor_free_buffers) {
        delete unencrypt_buf;
        delete xts_encrypt_buf;
        delete xts_in_aol;
        delete xts_out_aol;
        delete xts_desc_buf;
        delete xts_status_buf;
    }
}


/*
 * Execute any extra pre-push initialization
 */
void
encrypt_only_t::pre_push(encrypt_only_pre_push_params_t params)
{
    caller_xts_status_buf = params.caller_xts_status_buf_;
    caller_xts_opaque_buf = params.caller_xts_opaque_buf_;
    caller_xts_opaque_data = params.caller_xts_opaque_data_;
}

/*
 * Initiate the test
 */
int
encrypt_only_t::push(encrypt_only_push_params_t params)
{
    // validate app_blk_size
    if (params.app_blk_size_ > app_max_size) {
        printf("%s app_blk_size %u exceeds app_max_size %u\n", __FUNCTION__,
               params.app_blk_size_, app_max_size);
        assert(params.app_blk_size_ <= app_max_size);
        return -1;
    }

    if (!suppress_info_log) {
        printf("Starting testcase encrypt_only app_blk_size %u push_type %d "
               "seq_xts_qid %u\n", params.app_blk_size_, params.push_type_,
               params.seq_xts_qid_);
    }

    /*
     * Partially overwrite destination buffers to prevent left over
     * data from a previous run
     */
    xts_encrypt_buf->fragment_find(0, sizeof(uint64_t))->fill_thru(0xff);
    app_blk_size = params.app_blk_size_;
    success = false;

    // Set up encryption
    encrypt_setup(params.seq_xts_qid_);
    return 0;
}


/*
 * Execute any deferred push()
 */
void
encrypt_only_t::post_push(void)
{
    xts_ctx.ring_doorbell();
}


/*
 * Set up of XTS encryption
 */
void
encrypt_only_t::encrypt_setup(uint32_t seq_xts_qid)
{
    xts::xts_cmd_t  cmd;
    xts::xts_desc_t *xts_desc;

    // Use caller's XTS opaque info and status, if any
    if (caller_xts_opaque_buf) {
        xts_ctx.xts_db = caller_xts_opaque_buf;
        xts_ctx.xts_db_addr = 0;
        xts_ctx.exp_db_data = caller_xts_opaque_data;
        xts_ctx.caller_xts_db_en = true;
    }
    if (!caller_xts_status_buf) {
        caller_xts_status_buf = xts_status_buf;
    }
    xts_ctx.status = caller_xts_status_buf;
    xts_ctx.caller_status_en = true;

    // Calling xts_ctx init only to get its xts_db/status initialized
    xts_ctx.init(0, false);
    xts_ctx.suppress_info_log = suppress_info_log;
    xts_ctx.op = xts::AES_ENCR_ONLY;
    xts_ctx.seq_xts_q = seq_xts_qid;
    xts_ctx.push_type = ACC_RING_PUSH_SEQUENCER_BATCH;

    xts_in_aol->clear();
    xts::xts_aol_t *xts_in = (xts::xts_aol_t *)xts_in_aol->read();

    xts_in->a0 = unencrypt_buf->pa();
    xts_in->l0 = app_blk_size;
    xts_in_aol->write_thru();

    xts_out_aol->clear();
    xts::xts_aol_t *xts_out = (xts::xts_aol_t *)xts_out_aol->read();
    xts_out->a0 = xts_encrypt_buf->pa();
    xts_out->l0 = app_blk_size;
    xts_out_aol->write_thru();

    // Set up XTS encrypt descriptor
    xts_ctx.cmd_eval_seq_xts(cmd);
    xts_desc = (xts::xts_desc_t *)xts_desc_buf->read();
    xts_ctx.desc_prefill_seq_xts(xts_desc);
    xts_desc->in_aol = xts_in_aol->pa();
    xts_desc->out_aol = xts_out_aol->pa();
    xts_desc->cmd = cmd;
    xts_desc_buf->write_thru();
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
encrypt_only_t::fast_verify(void)
{
    // Validate XTS status
    xts::xts_status_t *xts_status =
         (xts::xts_status_t *)caller_xts_status_buf->read_thru();
    success = false;
    if (xts_status->status) {
      printf("ERROR: encrypt_only XTS error 0x%x\n", xts_status->status);
      return -1;
    }

    if (!suppress_info_log) {
        printf("Testcase encrypt_only fast_verify passed\n");
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
encrypt_only_t::full_verify(void)
{
    uint32_t    poll_factor = app_blk_size / kCompAppMinSize;

    // Verify XTS engine doorbell
    assert(poll_factor);
    success = false;

    if (xts_ctx.verify_doorbell(false, FLAGS_long_poll_interval * poll_factor)) {
        printf("ERROR: encrypt_only doorbell from XTS engine never came\n");
        return -1;
    }

    /*
     * Verify individual statuses
     */
    if (fast_verify()) {
        return -1;
    }
    success = false;

    // Status verification done.
    xts::xts_aol_t *xts_out = (xts::xts_aol_t *)xts_out_aol->read_thru();
    last_encrypt_output_data_len = xts_out->l0;

    if (!suppress_info_log) {
        xts_aol_trace("encrypt_only xts_out_aol", xts_out_aol,
                       xts_out_aol->num_lines_get(), true);
        printf("Testcase encrypt_only full_verify passed: "
               "last_encrypt_output_data_len %u\n", last_encrypt_output_data_len);
    }
    success = true;
    return 0;
}

}  // namespace tests
