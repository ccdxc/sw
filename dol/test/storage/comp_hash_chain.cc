// Accelerator compression to hash (for dedup) chaining DOLs.
#include <math.h>
#include "compression.hpp"
#include "compression_test.hpp"
#include "tests.hpp"
#include "utils.hpp"
#include "queues.hpp"

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
#include "comp_hash_chain.hpp"

namespace tests {


/*
 * Constructor
 */
comp_hash_chain_t::comp_hash_chain_t(comp_hash_chain_params_t params) :
    app_max_size(params.app_max_size_),
    app_blk_size(0),
    app_hash_size(kCompAppHashBlkSize),
    caller_comp_pad_buf(nullptr),
    caller_hash_status_vec(nullptr),
    caller_hash_opaque_vec(nullptr),
    caller_hash_opaque_data(0),
    comp_queue(nullptr),
    hash_queue(nullptr),
    last_cp_output_data_len(0),
    destructor_free_buffers(params.destructor_free_buffers_),
    suppress_info_log(params.suppress_info_log_),
    success(false)
{
    uncomp_buf = new dp_mem_t(1, app_max_size,
                              DP_MEM_ALIGN_PAGE, params.uncomp_mem_type_);

    // Size of compressed buffers accounts any for compression failure,
    // i.e., must be as large as uncompressed buffer plus cp_hdr_t.
    // 
    // Also, caller can elect to have 2 output buffers, e.g.
    // one in HBM for lower latency P4+ processing, and another in host
    // memory which P4+ will copy into for the application.
    comp_buf1 = new dp_mem_t(1, app_max_size + sizeof(cp_hdr_t),
                             DP_MEM_ALIGN_PAGE, params.comp_mem_type1_);
    if (params.comp_mem_type2_ != DP_MEM_TYPE_VOID) {
        comp_buf2 = new dp_mem_t(1, comp_buf1->line_size_get(),
                                 DP_MEM_ALIGN_NONE, params.comp_mem_type2_);
    } else {
        comp_buf2 = comp_buf1;
    }

    // for comp status, caller can elect to have 2 status buffers, e.g.
    // one in HBM for lower latency P4+ processing, and another in host
    // memory which P4+ will copy into for the application.
    comp_status_buf1 = new dp_mem_t(1, sizeof(cp_status_sha512_t),
                           DP_MEM_ALIGN_SPEC, params.comp_status_mem_type1_,
                           kMinHostMemAllocSize);
    if (params.comp_status_mem_type2_ != DP_MEM_TYPE_VOID) {
        comp_status_buf2 = new dp_mem_t(1, sizeof(cp_status_sha512_t),
                               DP_MEM_ALIGN_SPEC, params.comp_status_mem_type2_,
                               kMinHostMemAllocSize);
    } else {
        comp_status_buf2 = comp_status_buf1;
    }
    comp_opaque_buf = new dp_mem_t(1, sizeof(uint64_t),
                                   DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                   kMinHostMemAllocSize);
    /*
     * Allocate enough hash descriptors for the worst case
     */
    max_hash_blks = COMP_HASH_CHAIN_MAX_HASH_BLKS(app_max_size, sizeof(cp_hdr_t),
                                                  app_hash_size);
    hash_desc_vec = new dp_mem_t(max_hash_blks, sizeof(cp_desc_t),
                                 DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM,
                                 sizeof(cp_desc_t));
    hash_sgl_vec = new dp_mem_t(max_hash_blks, sizeof(cp_sgl_t),
                                DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM,
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
comp_hash_chain_t::~comp_hash_chain_t()
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
        if (comp_status_buf1 != comp_status_buf2) {
            delete comp_status_buf2;
        }
        delete comp_status_buf1;
        delete comp_opaque_buf;
        delete hash_desc_vec;
        delete hash_sgl_vec;
    }
}


/*
 * Execute any extra pre-push initialization
 */
void
comp_hash_chain_t::pre_push(comp_hash_chain_pre_push_params_t params)
{
    caller_comp_pad_buf = params.caller_comp_pad_buf_;
    caller_hash_status_vec = params.caller_hash_status_vec_;
    caller_hash_opaque_vec = params.caller_hash_opaque_vec_;
    caller_hash_opaque_data = params.caller_hash_opaque_data_;
}

/*
 * Initiate the test
 */
int 
comp_hash_chain_t::push(comp_hash_chain_push_params_t params)
{
    acc_chain_params_t  chain_params = {0};
    uint32_t            block_no;

    // validate app_blk_size
    if (!params.app_blk_size_ || (params.app_blk_size_ > app_max_size)) {
        printf("%s app_blk_size %u too small or exceeds app_max_size %u\n",
               __FUNCTION__, params.app_blk_size_, app_max_size);
        assert(params.app_blk_size_ && (params.app_blk_size_ <= app_max_size));
        return -1;
    }

    // validate app_hash_size_
    if (!params.app_hash_size_ || (params.app_hash_size_ > params.app_blk_size_) ||
        ((params.app_hash_size_ < kCompAppMinSize))) {
        printf("%s invalid app_hash_size_ %u in relation to app_blk_size %u "
               "and kCompAppMinSize %u\n", __FUNCTION__, params.app_hash_size_,
               params.app_blk_size_, kCompAppMinSize);
        assert(params.app_hash_size_ && (params.app_hash_size_ <= params.app_blk_size_) &&
               (params.app_hash_size_ >= kCompAppMinSize));
        return -1;
    }

    if (!suppress_info_log) {
        printf("Starting testcase comp_hash_chain app_blk_size %u app_hash_size %u "
               "push_type %d seq_comp_qid %u seq_comp_status_qid %u\n",
               params.app_blk_size_, params.app_hash_size_,
               params.push_type_, params.seq_comp_qid_,
               params.seq_comp_status_qid_);
    }

    /*
     * Calculate final (worst case) number of hash blocks required
     */
    app_blk_size = params.app_blk_size_;
    app_hash_size = params.app_hash_size_;
    num_hash_blks = COMP_HASH_CHAIN_MAX_HASH_BLKS(app_blk_size, sizeof(cp_hdr_t),
                                                  app_hash_size);
    if (num_hash_blks > max_hash_blks) {
        printf("%s num_hash_blks %u exceeds max_hash_blks %u\n",
               __FUNCTION__, num_hash_blks, max_hash_blks);
        assert(num_hash_blks <= max_hash_blks);
        return -1;
    }

    /*
     * Ensure caller supplied enough status buffers
     */
    if (caller_hash_status_vec->num_lines_get() < num_hash_blks) {
        printf("%s number of status buffers %u is less than num_hash_blks %u\n",
               __FUNCTION__, caller_hash_status_vec->num_lines_get(),
               num_hash_blks);
        assert(caller_hash_status_vec->num_lines_get() >= num_hash_blks);
        return -1;
    }

    /*
     * Hash interrupts are optional, depending on what the application wants
     */
    if (caller_hash_opaque_vec &&
        (caller_hash_opaque_vec->num_lines_get() < num_hash_blks)) {
        printf("%s number of status buffers %u is less than num_hash_blks %u\n",
               __FUNCTION__, caller_hash_opaque_vec->num_lines_get(),
               num_hash_blks);
        assert(caller_hash_opaque_vec->num_lines_get() >= num_hash_blks);
        return -1;
    }
    
    /*
     * Capture hash params
     */
    sha_en = params.sha_en_;
    sha_type = params.sha_type_;
    integrity_src = params.integrity_src_;
    integrity_type = params.integrity_type_;

    /*
     * Partially overwrite destination buffers to prevent left over
     * data from a previous run
     */
    comp_buf1->fragment_find(0, 64)->fill_thru(0xff);
    comp_buf2->fragment_find(0, 64)->fill_thru(0xff);

    comp_queue = params.comp_queue_;
    hash_queue = params.hash_queue_;
    success = false;
    compress_cp_desc_template_fill(cp_desc, uncomp_buf, comp_buf1,
                     comp_status_buf1, comp_buf1, app_blk_size);
    /*
     * point barco_desc_addr to the first of the descriptors vector,
     * and do the same for the hash SGL vector
     */
    hash_desc_vec->line_set(0);
    hash_sgl_vec->line_set(0);

    /* 
     * Hash chaining will use direct Barco push action from
     * comp status queue handler. Hence, no seq_next_q needed.
     */
    chain_params.desc_format_fn = test_setup_post_comp_seq_status_entry;
    chain_params.seq_q = params.seq_comp_qid_;
    chain_params.seq_status_q = params.seq_comp_status_qid_;
    chain_params.chain_ent.next_doorbell_en = 1;
    chain_params.chain_ent.next_db_action_barco_push = 1;
    chain_params.chain_ent.push_entry.barco_ring_addr = hash_queue->q_base_mem_pa_get();
    chain_params.chain_ent.push_entry.barco_pndx_addr = hash_queue->cfg_q_pd_idx_get();
    chain_params.chain_ent.push_entry.barco_pndx_shadow_addr = hash_queue->shadow_pd_idx_pa_get();
    chain_params.chain_ent.push_entry.barco_desc_size =
                           (uint8_t)log2(hash_desc_vec->line_size_get());
    chain_params.chain_ent.push_entry.barco_pndx_size =
                           (uint8_t)log2(sizeof(uint32_t));
    chain_params.chain_ent.push_entry.barco_ring_size = 
                           (uint8_t)log2(hash_queue->q_size_get());
    chain_params.chain_ent.push_entry.barco_desc_addr = hash_desc_vec->pa();
    chain_params.chain_ent.sgl_vec_pa = hash_sgl_vec->pa();

    comp_status_buf1->clear_thru();
    comp_status_buf2->clear_thru();
    chain_params.chain_ent.status_hbm_pa = comp_status_buf1->pa();
    if (comp_status_buf1 != comp_status_buf2) {
        chain_params.chain_ent.status_dma_en = 1;
        chain_params.chain_ent.status_host_pa = comp_status_buf2->pa();
        chain_params.chain_ent.status_len = comp_status_buf2->line_size_get();
    }

    chain_params.chain_ent.pad_buf_pa = caller_comp_pad_buf->pa();
    chain_params.chain_ent.stop_chain_on_error = 1;
    chain_params.chain_ent.sgl_pad_hash_en = 1;
    chain_params.chain_ent.pad_len_shift =
                 (uint8_t)log2(caller_comp_pad_buf->line_size_get());

    // Enable interrupt in case compression fails
    comp_opaque_buf->clear_thru();
    chain_params.chain_ent.intr_pa = comp_opaque_buf->pa();
    chain_params.chain_ent.intr_data = kCompSeqIntrData;
    chain_params.chain_ent.intr_en = 1;

    /*
     * If only a max of 1 hash block is required, compression plus hash/checksum
     * can all be done in one Barco request. Question: are hash/checksum results
     * for a block (of less than 4K) identical to the result for the same block padded
     * to 4K? For now, let's assume they're not and execute the hash/checksum
     * operations separately from the comp operation.
     */
    for (block_no = 0; block_no < num_hash_blks; block_no++) {
        hash_setup(block_no, chain_params);
    }

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
comp_hash_chain_t::post_push(void)
{
    comp_queue->post_push();
}


/*
 * Set up of hash
 */
void 
comp_hash_chain_t::hash_setup(uint32_t block_no,
                              acc_chain_params_t& chain_params)
{
    cp_desc_t   *hash_desc;

    hash_sgl_vec->line_set(block_no);
    hash_desc_vec->line_set(block_no);
    caller_hash_status_vec->line_set(block_no);

    hash_sgl_vec->clear();
    hash_desc_vec->clear();
    caller_hash_status_vec->clear_thru();

    // Note that hash is a follow-on operation after compression.
    // For this to work, the cfg_ueng CSR must have sha_data_src
    // set to 1 which, even though it indicates "use uncompressed data for SHA",
    // the input is still compressed data because the hash is executed
    // as a separate pass.
    //
    // If the above guideline is not followed, the model will throw
    // "[ERROR] cmp_en = 0 and sha_en = 1 and sha_data_src = 0 is illegal"
    
    hash_desc = (cp_desc_t *)hash_desc_vec->read();
    hash_desc->cmd_bits.sha_en = sha_en;
    hash_desc->cmd_bits.sha_type = sha_type;
    hash_desc->cmd_bits.integrity_src = integrity_src;
    hash_desc->cmd_bits.integrity_type = integrity_type;

    /*
     * Set up the hash src as SGL.
     * Note: p4+ will modify len0 and addr1/len1 below based on compression
     * output_data_len and any required padding
     */
    cp_sgl_t *hash_sgl = (cp_sgl_t *)hash_sgl_vec->read();
    hash_sgl->addr0 = comp_buf1->pa() + (block_no * app_hash_size);
    hash_sgl->len0 = app_hash_size;
    hash_sgl_vec->write_thru();

    hash_desc->cmd_bits.src_is_list = 1;
    hash_desc->src = hash_sgl_vec->pa();
    hash_desc->datain_len = app_hash_size;
    hash_desc->threshold_len = app_hash_size;
    hash_desc->status_addr = caller_hash_status_vec->pa();
    hash_desc->status_data = 0x9abc;

    /*
     * Hash interrupts are optional, depending on what the application wants
     */
    if (caller_hash_opaque_vec) {
        caller_hash_opaque_vec->line_set(block_no);
        caller_hash_opaque_vec->clear_thru();

        hash_desc->opaque_tag_addr = caller_hash_opaque_vec->pa();
        hash_desc->opaque_tag_data = caller_hash_opaque_data;
        hash_desc->cmd_bits.opaque_tag_on = 1;
    }
    hash_desc_vec->write_thru();
}


/*
 * Test result verification
 */
int 
comp_hash_chain_t::verify(void)
{
    cp_desc_t   *hash_desc;
    uint32_t    actual_hash_blks;
    uint32_t    block_no;

    // Poll for comp status
    if (!comp_status_poll(comp_status_buf2, suppress_info_log)) {
      printf("ERROR: comp_hash_chain compression status never came\n");
      return -1;
    }

    // Validate comp status
    if (compress_status_verify(comp_status_buf2, comp_buf1, cp_desc)) {
        printf("ERROR: comp_hash_chain compression status verification failed\n");
        return -1;
    }
    last_cp_output_data_len = comp_status_output_data_len_get(comp_status_buf2);

    // Verify all hash status
    actual_hash_blks = COMP_HASH_CHAIN_MAX_HASH_BLKS(last_cp_output_data_len, 0,
                                                     app_hash_size);
    if (!suppress_info_log) {
        printf("comp_hash_chain: last_cp_output_data_len %u actual_hash_blks %u\n",
               last_cp_output_data_len, actual_hash_blks);
    }
    if ((actual_hash_blks == 0) || (actual_hash_blks > num_hash_blks)) {
        printf("comp_hash_chain: invalid actual_hash_blks %u in relation to "
               "num_hash_blks %u\n", actual_hash_blks, num_hash_blks);
        return -1;
    }
    for (block_no = 0; block_no < actual_hash_blks; block_no++) {
        caller_hash_status_vec->line_set(block_no);
        hash_desc_vec->line_set(block_no);
        hash_desc = (cp_desc_t *)hash_desc_vec->read_thru();

        if (!comp_status_poll(caller_hash_status_vec, suppress_info_log)) {
            printf("ERROR: comp_hash_chain block %u hash status never came\n",
                   block_no);
            return -1;
        }

        if (compress_status_verify(caller_hash_status_vec, comp_buf1, *hash_desc)) {
            printf("ERROR: comp_hash_chain hash block %u status verification failed\n",
                   block_no);
            return -1;
        }
    }

    if (!suppress_info_log) {
        printf("Testcase comp_hash_chain passed\n");
    }
    success = true;
    return 0;
}

}  // namespace tests
