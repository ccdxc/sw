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
    actual_hash_blks(-1),
    seq_sgl_pdma(nullptr),
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
    // memory which P4+ will PDMA transfer into for the application.
    comp_buf1 = new dp_mem_t(1, app_max_size + sizeof(cp_hdr_t),
                             DP_MEM_ALIGN_PAGE, params.comp_mem_type1_);
    if (params.comp_mem_type2_ != DP_MEM_TYPE_VOID) {
        comp_buf2 = new dp_mem_t(1, comp_buf1->line_size_get(),
                                 DP_MEM_ALIGN_PAGE, params.comp_mem_type2_);
    } else {
        comp_buf2 = comp_buf1;
    }
    seq_sgl_pdma = new dp_mem_t(1, sizeof(cp_sq_ent_sgl_t),
                                DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HBM,
                                sizeof(cp_sq_ent_sgl_t));

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
        delete seq_sgl_pdma;
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
    cp_sq_ent_sgl_t     *sgl_pdma_entry;
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
    actual_hash_blks = -1;
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

    /*
     * P4+ is capable of initiating chained hash operation simultaneous
     * with PDMA transfer of the comp data. So activate that if the caller
     * had requested it.
     */
    if (comp_buf1 != comp_buf2) {
        seq_sgl_pdma->clear();
        sgl_pdma_entry = (cp_sq_ent_sgl_t *)seq_sgl_pdma->read();
        sgl_pdma_entry->addr[0] = comp_buf2->pa();
        sgl_pdma_entry->len[0] = comp_buf2->line_size_get();
        seq_sgl_pdma->write_thru();

        chain_params.chain_ent.dst_hbm_pa = comp_buf1->pa();
        chain_params.chain_ent.sgl_pdma_out_pa = seq_sgl_pdma->pa();
        chain_params.chain_ent.sgl_pdma_en = 1;
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
     * If only one hash block is required, it is possible to do compression
     * plus hash with one Barco request. However, doing so would require setting
     * sha_data_src to 0 in the cfg_ueng CSR, which would conflict with other
     * requests that require multiple hash blocks. Hence, we normalize this
     * by always executing hash separately from the comp operation.
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
        if (!suppress_info_log) {
            printf("comp_hash_chain HW to possibly write hash opaque_tag_addr 0x%lx "
                   "opaque_tag_data 0x%x\n", hash_desc->opaque_tag_addr,
                   hash_desc->opaque_tag_data);
        }
    }
    hash_desc_vec->write_thru();
}


/*
 * Return actual number of hash blocks based on the output data length
 * from compression.
 */
int 
comp_hash_chain_t::actual_hash_blks_get(comp_hash_chain_retrieve_method_t method)
{
    bool    log_error = (method == COMP_HASH_CHAIN_BLOCKING_RETRIEVE);

    if (actual_hash_blks < 0) {

        switch (method) {

        case COMP_HASH_CHAIN_BLOCKING_RETRIEVE:

            // Poll for comp status
            if (!comp_status_poll(comp_status_buf2, suppress_info_log)) {
              printf("ERROR: comp_hash_chain compression status never came\n");
              return -1;
            }

            /*
             * Fall through!!!
             */
             
        case COMP_HASH_CHAIN_NON_BLOCKING_RETRIEVE:
        default:

            // Validate comp status
            if (compress_status_verify(comp_status_buf2, comp_buf1, cp_desc, log_error)) {
                if (log_error) {
                    printf("ERROR: comp_hash_chain compression status verification failed\n");
                }
                return -1;
            }
            last_cp_output_data_len = comp_status_output_data_len_get(comp_status_buf2);
            actual_hash_blks = (int)COMP_HASH_CHAIN_MAX_HASH_BLKS(last_cp_output_data_len, 0,
                                                                  app_hash_size);
            if (!suppress_info_log) {
                printf("comp_hash_chain: last_cp_output_data_len %u actual_hash_blks %u\n",
                       last_cp_output_data_len, actual_hash_blks);
            }
            if ((actual_hash_blks == 0) || ((uint32_t)actual_hash_blks > num_hash_blks)) {
                printf("comp_hash_chain: invalid actual_hash_blks %d in relation to "
                       "num_hash_blks %u\n", actual_hash_blks, num_hash_blks);
                actual_hash_blks = -1;
            }
        }
    }

    return actual_hash_blks;
}


/*
 * Test result verification
 */
int 
comp_hash_chain_t::verify(void)
{
    cp_desc_t   *hash_desc;
    cp_sgl_t    *hash_sgl;
    uint32_t    block_no;
    uint32_t    pad_data_len;
    uint64_t    total_data_len;
    uint64_t    accum_data_len0;
    uint64_t    accum_data_len1;
    uint64_t    accum_data_len2;
    uint64_t    accum_link;
    uint64_t    total_accum_data_len;

    if (actual_hash_blks_get(COMP_HASH_CHAIN_BLOCKING_RETRIEVE) < 0) {
        return -1;
    }

    /*
     * Verify all hash status and P4+ modified SGL lengths
     */
    total_data_len = actual_hash_blks * app_hash_size;
    accum_data_len0 = 0;
    accum_data_len1 = 0;
    accum_data_len2 = 0;
    accum_link = 0;
    pad_data_len = 0;

    for (block_no = 0; block_no < (uint32_t)actual_hash_blks; block_no++) {
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

        /*
         * Accumulate SGL lengths to be validated
         */
        hash_sgl_vec->line_set(block_no);
        hash_sgl = (cp_sgl_t *)hash_sgl_vec->read_thru();
        accum_data_len0 += hash_sgl->len0;
        accum_data_len1 += hash_sgl->len1;
        accum_data_len2 += hash_sgl->len2;
        accum_link |= hash_sgl->link;
        if (block_no == ((uint32_t)actual_hash_blks - 1)) {
            pad_data_len = hash_sgl->len1;
        }
    }

    /*
     * Validate total accumulated length
     */
    total_accum_data_len = accum_data_len0 + accum_data_len1 + accum_data_len2;
    if (total_accum_data_len != total_data_len) {
        printf("ERROR: comp_hash_chain total_accum_data_len %lu expected %lu\n",
               total_accum_data_len, total_data_len);
        return -1;
    }

    /*
     * Validate padding length
     */
    if (pad_data_len != (total_data_len - last_cp_output_data_len)) {
        printf("ERROR: comp_hash_chain invalid pad_len %u in relation to "
               "total_data_len %lu and last_cp_output_data_len %u\n",
               pad_data_len, total_data_len, last_cp_output_data_len);
        return -1;
    }

    /*
     * Validate other lengths
     */
    if (pad_data_len != accum_data_len1) {
        printf("ERROR: comp_hash_chain invalid pad_len %u in relation to "
               "accum_data_len1 %lu\n", pad_data_len, accum_data_len1);
        return -1;
    }

    if (accum_data_len2 || accum_link) {
        printf("ERROR: comp_hash_chain unexpected accum_data_len2 %lu "
               "or accum_link %lu\n", accum_data_len2, accum_link);
        return -1;
    }

    /*
     * Validate PDMA transfer capability
     */
    if (comp_buf1 != comp_buf2) {
        if (test_data_verify_and_dump(comp_buf1->read_thru(),
                                      comp_buf2->read_thru(),
                                      last_cp_output_data_len)) {
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
