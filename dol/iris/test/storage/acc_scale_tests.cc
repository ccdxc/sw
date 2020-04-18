// Accelerator scale DOLs.
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
#include "acc_scale_tests.hpp"

namespace tests {


/*************************************************************************************/
/* Accelerator compression to XTS-encrypt chaining scale                             */
/*************************************************************************************/

/*
 * Accelerator compression to XTS-encrypt chaining scale
 * Constructor
 */
comp_encrypt_chain_scale_t::comp_encrypt_chain_scale_t(comp_encrypt_chain_scale_params_t params) :
    scale_test_name(__FUNCTION__),
    destructor_free_buffers(params.destructor_free_buffers_),
    success(false)
{
    comp_encrypt_chain_pre_push_params_t pre_push;
    dp_mem_t    *curr_xts_status_host;
    dp_mem_t    *curr_xts_opaque_host;
    uint32_t    max_enc_blks;
    uint32_t    i;

    // Instantiate unique comp_encrypt_chain_t tests one by one, as opposed to
    // using vector instantiation since it would use the same value for all
    // instances which is not what we want.
    assert(params.num_chains_);
    for (i = 0; i < params.num_chains_; i++) {
        comp_encrypt_chain_vec.push_back(new comp_encrypt_chain_t(params.cec_params_));
    }

    // Allocate status and opaque tags as a byte streams for fast memcmp.
    max_enc_blks = COMP_MAX_HASH_BLKS(params.cec_params_.app_max_size_,
                                      params.cec_params_.app_enc_size_);
    exp_opaque_data_buf = new dp_mem_t(1, max_enc_blks * sizeof(uint64_t),
                                       DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
    exp_opaque_data = 0xcccccccccccccccc;
    exp_opaque_data_buf->fill_thru(0xcc);

    // Other inits

    for (i = 0; i < comp_encrypt_chain_vec.size(); i++) {
        curr_xts_status_host = new dp_mem_t(max_enc_blks, sizeof(xts::xts_status_t),
                                   DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM, sizeof(xts::xts_status_t));
        curr_xts_opaque_host = new dp_mem_t(max_enc_blks, sizeof(uint64_t),
                                   DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM, sizeof(uint64_t));
        comp_encrypt_chain_vec[i]->pre_push(pre_push.caller_comp_pad_buf(comp_pad_buf).
                                                     caller_xts_status_vec(curr_xts_status_host).
                                                     caller_xts_opaque_vec(curr_xts_opaque_host).
                                                     caller_xts_opaque_data(exp_opaque_data));
        xts_status_host_vec.push_back(curr_xts_status_host);
        xts_opaque_host_vec.push_back(curr_xts_opaque_host);
    }
}


/*
 * Accelerator compression to XTS-encrypt chaining scale
 * Destructor
 */
comp_encrypt_chain_scale_t::~comp_encrypt_chain_scale_t()
{
    uint32_t    i;

    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.

    printf("%s success %u destructor_free_buffers %u\n",
           __FUNCTION__, success, destructor_free_buffers);
    for (i = 0; i < comp_encrypt_chain_vec.size(); i++) {
        delete comp_encrypt_chain_vec[i];
    }

    if (success && destructor_free_buffers) {
        for (i = 0; i < comp_encrypt_chain_vec.size(); i++) {
            delete xts_status_host_vec[i];
            delete xts_opaque_host_vec[i];
        }
        xts_status_host_vec.clear();
        xts_opaque_host_vec.clear();
        delete exp_opaque_data_buf;
    }
    comp_encrypt_chain_vec.clear();
}


/*
 * Accelerator compression to XTS-encrypt chaining scale
 * Establish initial push parameters
 */
void
comp_encrypt_chain_scale_t::push_params_set(comp_encrypt_chain_push_params_t params)
{
    push_params = params;
}


/*
 * Accelerator compression to XTS-encrypt chaining scale
 * Set up the test
 */
int
comp_encrypt_chain_scale_t::push(void)
{
    uint32_t    initial_status_sq = push_params.seq_comp_status_qid_;
    uint32_t    i;

    success = false;
    for (i = 0; i < comp_encrypt_chain_vec.size(); i++) {
        xts_status_host_vec[i]->all_lines_clear_thru();

        // Encryption is last in chain so does not require status sequencer
        push_params.seq_xts_status_qid_ = 0;

        // Push all entries on the same compression SQ but each submission
        // must be handled with its own comp status SQ since the completion
        // order will not be known.
        comp_encrypt_chain_vec[i]->push(push_params);
        push_params.seq_comp_status_qid_++;
    }

    push_params.seq_comp_status_qid_ = initial_status_sq;
    return 0;
}


/*
 * Accelerator compression to XTS-encrypt chaining scale
 * Initiate the test
 */
int
comp_encrypt_chain_scale_t::post_push(void)
{
    uint32_t    num_submissions = comp_encrypt_chain_vec.size();

    /*
     * Only need to call post_push() once since all submissions went
     * to the same comp_ring.
     */
    assert(num_submissions);
    if (num_submissions) {
        comp_encrypt_chain_vec[num_submissions - 1]->post_push();
        run_count++;
    }
    return 0;
}


/*
 * Accelerator compression to XTS-encrypt chaining scale
 * Check for test completion and return 0 if so.
 *
 * Note: completion_check() must be non-blocking, i.e., do not use
 * tests::Poller within this function or any descendant functions.
 */
int
comp_encrypt_chain_scale_t::completion_check(void)
{
    int         actual_enc_blks;
    uint32_t    i;

    success = true;
    verification_time_advance();
    for (i = 0; i < comp_encrypt_chain_vec.size(); i++) {
        actual_enc_blks =
            comp_encrypt_chain_vec[i]->actual_enc_blks_get(TEST_RESOURCE_NON_BLOCKING_QUERY);
        xts_opaque_host_vec[i]->line_set(0);
        if ((actual_enc_blks < 0) ||
            memcmp(xts_opaque_host_vec[i]->read_thru(),
                   exp_opaque_data_buf->read_thru(),
                   actual_enc_blks * sizeof(uint64_t))) {
            success = false;
            break;
        }
    }

    return success ? 0 : -1;
}


/*
 * Accelerator compression to XTS-encrypt chaining scale
 * Encryption is the last step in the chain so do a fast verification
 * of all XTS status entries.
 */
int
comp_encrypt_chain_scale_t::fast_verify(void)
{
    uint32_t    i;

    success = true;
    for (i = 0; i < comp_encrypt_chain_vec.size(); i++) {
        if (comp_encrypt_chain_vec[i]->fast_verify()) {
            printf("ERROR: %s submission #%u failed\n",
                   __FUNCTION__, i);
            success = false;
        }
    }

    return success ? 0 : -1;
}

/*
 * Accelerator compression to XTS-encrypt chaining scale
 * Perform a slower but full verification of all chain results.
 */
int
comp_encrypt_chain_scale_t::full_verify(void)
{
    uint32_t    i;

    success = true;
    for (i = 0; i < comp_encrypt_chain_vec.size(); i++) {
        if (comp_encrypt_chain_vec[i]->full_verify()) {
            printf("ERROR: %s submission #%u failed\n",
                   __FUNCTION__, i);
            success = false;
        }
    }

    return success ? 0 : -1;
}


/*************************************************************************************/
/* Accelerator XTS-decrypt to decompression chaining scale                           */
/*************************************************************************************/

/*
 * Accelerator XTS-decrypt to decompression chaining scale
 * Constructor
 */
decrypt_decomp_chain_scale_t::decrypt_decomp_chain_scale_t(decrypt_decomp_chain_scale_params_t params) :
    scale_test_name(__FUNCTION__),
    destructor_free_buffers(params.destructor_free_buffers_),
    success(false)
{
    decrypt_decomp_chain_pre_push_params_t pre_push;
    dp_mem_t    *comp_status_frag;
    dp_mem_t    *comp_opaque_frag;
    uint32_t    i;

    // Instantiate unique comp_encrypt_chain_t tests one by one, as opposed to
    // using vector instantiation since it would use the same value for all
    // instances which is not what we want.
    assert(params.num_chains_);
    for (i = 0; i < params.num_chains_; i++) {
        decrypt_decomp_chain_vec.push_back(new decrypt_decomp_chain_t(params.ddc_params_));
    }

    // Comp status is a multi-field structure so byte stream handling is not possible
    comp_status_host_buf = new dp_mem_t(params.num_chains_, sizeof(cp_status_sha512_t),
                               DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);

    // Allocate opaque tags as a byte streams for fast memcmp
    comp_opaque_host_buf = new dp_mem_t(1,  params.num_chains_ * sizeof(uint64_t),
                               DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM, sizeof(uint64_t));
    exp_opaque_data_buf = new dp_mem_t(1, params.num_chains_ * sizeof(uint64_t),
                                       DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);

    // Note that RTL expects each opaque tag as uint64_t and writes
    // exp_opaque_data to the first 4 bytes, followed by 0 in the next 4 bytes
    exp_opaque_data = 0xdddddddd;
    for (i = 0; i < decrypt_decomp_chain_vec.size(); i++) {
        memcpy(exp_opaque_data_buf->read() + (i * sizeof(uint64_t)),
               &exp_opaque_data, sizeof(uint32_t));
    }
    exp_opaque_data_buf->write_thru();

    // Other inits

    for (i = 0; i < decrypt_decomp_chain_vec.size(); i++) {
        comp_status_host_buf->line_set(i);
        comp_status_frag = comp_status_host_buf->fragment_find(0, sizeof(cp_status_sha512_t));
        comp_opaque_frag = comp_opaque_host_buf->fragment_find(i * sizeof(uint64_t),
                                                               sizeof(uint64_t));
        decrypt_decomp_chain_vec[i]->pre_push(pre_push.caller_comp_status_buf(comp_status_frag).
                                                       caller_comp_opaque_buf(comp_opaque_frag).
                                                       caller_comp_opaque_data(exp_opaque_data));
    }
}


/*
 * Accelerator XTS-decrypt to decompression chaining scale
 * Destructor
 */
decrypt_decomp_chain_scale_t::~decrypt_decomp_chain_scale_t()
{
    uint32_t    i;

    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.

    printf("%s success %u destructor_free_buffers %u\n",
           __FUNCTION__, success, destructor_free_buffers);
    for (i = 0; i < decrypt_decomp_chain_vec.size(); i++) {
        delete decrypt_decomp_chain_vec[i];
    }
    decrypt_decomp_chain_vec.clear();

    if (success && destructor_free_buffers) {
        delete comp_status_host_buf;
        delete comp_opaque_host_buf;
        delete exp_opaque_data_buf;
    }
}


/*
 * Accelerator XTS-decrypt to decompression chaining scale
 * Establish initial push parameters
 */
void
decrypt_decomp_chain_scale_t::push_params_set(decrypt_decomp_chain_push_params_t params)
{
    push_params = params;
}


/*
 * Accelerator XTS-decrypt to decompression chaining scale
 * Set up the test
 */
int
decrypt_decomp_chain_scale_t::push(void)
{
    uint32_t    initial_status_sq = push_params.seq_xts_status_qid_;
    uint32_t    i;

    success = false;
    comp_opaque_host_buf->clear_thru();

    for (i = 0; i < decrypt_decomp_chain_vec.size(); i++) {

        // Decompression is last in chain so does not require status sequencer
        push_params.seq_comp_status_qid_ = 0;

        // Push all entries on the same compression SQ but each submission
        // must be handled with its own XTS status SQ since the completion
        // order will not be known.
        decrypt_decomp_chain_vec[i]->push(push_params);
        push_params.seq_xts_status_qid_++;
    }

    push_params.seq_xts_status_qid_ = initial_status_sq;
    return 0;
}


/*
 * Accelerator XTS-decrypt to decompression chaining scale
 * Initiate the test
 */
int
decrypt_decomp_chain_scale_t::post_push(void)
{
    uint32_t    num_submissions = decrypt_decomp_chain_vec.size();

    /*
     * Only need to call post_push() once since all submissions went
     * to the same XTS sequencer queue.
     */
    assert(num_submissions);
    if (num_submissions) {
        decrypt_decomp_chain_vec[num_submissions - 1]->post_push();
        run_count++;
    }
    return 0;
}


/*
 * Accelerator XTS-decrypt to decompression chaining scale
 * Check for test completion and return 0 if so.
 *
 * Note: completion_check() must be non-blocking, i.e., do not use
 * tests::Poller within this function or any descendant functions.
 */
int
decrypt_decomp_chain_scale_t::completion_check(void)
{
    verification_time_advance();
    success = memcmp(comp_opaque_host_buf->read_thru(),
                     exp_opaque_data_buf->read_thru(),
                     comp_opaque_host_buf->line_size_get()) == 0;
    return success ? 0 : -1;
}


/*
 * Accelerator XTS-decrypt to decompression chaining scale
 * Comp status is a multi-field structure and for fast verification,
 * just check the valid and error bits.
 */
int
decrypt_decomp_chain_scale_t::fast_verify(void)
{
    uint32_t            i;

    success = true;
    for (i = 0; i < decrypt_decomp_chain_vec.size(); i++) {
        if (decrypt_decomp_chain_vec[i]->fast_verify()) {
            printf("ERROR: %s submission #%u failed\n",
                   __FUNCTION__, i);
            success = false;
        }
    }

    return success ? 0 : -1;
}


/*
 * Accelerator XTS-decrypt to decompression chaining scale
 * Perform a slower but full verification of all chain results.
 */
int
decrypt_decomp_chain_scale_t::full_verify(void)
{
    uint32_t    i;

    success = true;
    for (i = 0; i < decrypt_decomp_chain_vec.size(); i++) {
        if (decrypt_decomp_chain_vec[i]->full_verify()) {
            printf("ERROR: %s submission #%u failed\n",
                   __FUNCTION__, i);
            success = false;
        }
    }

    return success ? 0 : -1;
}


/*************************************************************************************/
/* Accelerator compression to hash chaining scale                                    */
/*************************************************************************************/

/*
 * Accelerator compression to hash chaining scale
 * Constructor
 */
comp_hash_chain_scale_t::comp_hash_chain_scale_t(comp_hash_chain_scale_params_t params) :
    scale_test_name(__FUNCTION__),
    destructor_free_buffers(params.destructor_free_buffers_),
    success(false)
{
    comp_hash_chain_pre_push_params_t pre_push;
    dp_mem_t    *curr_hash_status_host;
    dp_mem_t    *curr_hash_opaque_host;
    uint32_t    max_hash_blks;
    uint32_t    i;

    assert(params.num_chains_);
    for (i = 0; i < params.num_chains_; i++) {
        comp_hash_chain_vec.push_back(new comp_hash_chain_t(params.chc_params_));
    }

    // Note that RTL expects each opaque tag as uint64_t and writes
    // exp_opaque_data to the first 4 bytes, followed by 0 in the next 4 bytes
    max_hash_blks = COMP_MAX_HASH_BLKS(params.chc_params_.app_max_size_,
                                       kCompAppHashBlkSize);
    exp_opaque_data_buf = new dp_mem_t(1, max_hash_blks * sizeof(uint64_t),
                                       DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                       kMinHostMemAllocSize);
    exp_opaque_data = 0xffeeddcc;
    for (i = 0; i < max_hash_blks; i++) {
        memcpy(exp_opaque_data_buf->read() + (i * sizeof(uint64_t)),
               &exp_opaque_data, sizeof(uint32_t));
    }
    exp_opaque_data_buf->write_thru();

    // Other inits

    for (i = 0; i < comp_hash_chain_vec.size(); i++) {

        // Hash is multi-block per chain and requires a vector of status vectors,
        // and a vector of opaque vectors!
        curr_hash_status_host = new dp_mem_t(max_hash_blks, CP_STATUS_PAD_ALIGNED_SIZE,
                                             DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                             kMinHostMemAllocSize);
        curr_hash_opaque_host = new dp_mem_t(max_hash_blks, sizeof(uint64_t),
                                             DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                             kMinHostMemAllocSize);
        comp_hash_chain_vec[i]->pre_push(pre_push.caller_comp_pad_buf(comp_pad_buf).
                                                  caller_hash_status_vec(curr_hash_status_host).
                                                  caller_hash_opaque_vec(curr_hash_opaque_host).
                                                  caller_hash_opaque_data(exp_opaque_data));
        hash_status_host_vec.push_back(curr_hash_status_host);
        hash_opaque_host_vec.push_back(curr_hash_opaque_host);
    }
}


/*
 * Accelerator compression to hash chaining scale
 * Destructor
 */
comp_hash_chain_scale_t::~comp_hash_chain_scale_t()
{
    uint32_t    i;

    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.

    printf("%s success %u destructor_free_buffers %u\n",
           __FUNCTION__, success, destructor_free_buffers);
    for (i = 0; i < comp_hash_chain_vec.size(); i++) {
        delete comp_hash_chain_vec[i];
    }

    if (success && destructor_free_buffers) {

        for (i = 0; i < comp_hash_chain_vec.size(); i++) {
            delete hash_status_host_vec[i];
            delete hash_opaque_host_vec[i];
        }
        hash_status_host_vec.clear();
        hash_opaque_host_vec.clear();
        delete exp_opaque_data_buf;
    }
    comp_hash_chain_vec.clear();
}


/*
 * Accelerator compression to hash chaining scale
 * Establish initial push parameters
 */
void
comp_hash_chain_scale_t::push_params_set(comp_hash_chain_push_params_t params)
{
    push_params = params;
}


/*
 * Accelerator compression to hash chaining scale
 * Set up the test
 */
int
comp_hash_chain_scale_t::push(void)
{
    uint32_t    initial_status_sq = push_params.seq_comp_status_qid_;
    uint32_t    i;

    success = false;
    for (i = 0; i < comp_hash_chain_vec.size(); i++) {
        hash_opaque_host_vec[i]->all_lines_clear_thru();

        // Push all entries on the same compression SQ but each submission
        // must be handled with its own comp status SQ since the completion
        // order will not be known.
        comp_hash_chain_vec[i]->push(push_params);
        push_params.seq_comp_status_qid_++;
    }

    push_params.seq_comp_status_qid_ = initial_status_sq;
    return 0;
}


/*
 * Accelerator compression to hash chaining scale
 * Initiate the test
 */
int
comp_hash_chain_scale_t::post_push(void)
{
    uint32_t    num_submissions = comp_hash_chain_vec.size();

    /*
     * Only need to call post_push() once since all submissions went
     * to the same comp_ring.
     */
    assert(num_submissions);
    if (num_submissions) {
        comp_hash_chain_vec[num_submissions - 1]->post_push();
        run_count++;
    }
    return 0;
}


/*
 * Accelerator compression to hash chaining scale
 * Check for test completion and return 0 if so.
 *
 * Note: completion_check() must be non-blocking, i.e., do not use
 * tests::Poller within this function or any descendant functions.
 */
int
comp_hash_chain_scale_t::completion_check(void)
{
    int         actual_hash_blks;
    uint32_t    i;

    success = true;
    verification_time_advance();
    for (i = 0; i < comp_hash_chain_vec.size(); i++) {
        actual_hash_blks =
            comp_hash_chain_vec[i]->actual_hash_blks_get(TEST_RESOURCE_NON_BLOCKING_QUERY);
        hash_opaque_host_vec[i]->line_set(0);
        if ((actual_hash_blks < 0) ||
            memcmp(hash_opaque_host_vec[i]->read_thru(),
                   exp_opaque_data_buf->read_thru(),
                   actual_hash_blks * sizeof(uint64_t))) {
            success = false;
            break;
        }
    }

    return success ? 0 : -1;
}


/*
 * Accelerator compression to hash chaining scale
 */
int
comp_hash_chain_scale_t::fast_verify(void)
{
    uint32_t    i;

    success = true;
    for (i = 0; i < comp_hash_chain_vec.size(); i++) {
        if (comp_hash_chain_vec[i]->fast_verify()) {
            printf("ERROR: %s submission #%u failed\n",
                   __FUNCTION__, i);
            success = false;
        }
    }

    return success ? 0 : -1;
}

/*
 * Accelerator compression to hash chaining scale
 * Perform a slower but full verification of all chain results.
 */
int
comp_hash_chain_scale_t::full_verify(void)
{
    uint32_t    i;

    success = true;
    for (i = 0; i < comp_hash_chain_vec.size(); i++) {
        if (comp_hash_chain_vec[i]->full_verify()) {
            printf("ERROR: %s submission #%u failed\n",
                   __FUNCTION__, i);
            success = false;
        }
    }

    return success ? 0 : -1;
}


/*************************************************************************************/
/* Accelerator checksum to decompression chaining scale                              */
/*************************************************************************************/

/*
 * Accelerator checksum to decompression chaining scale
 * Constructor
 */
chksum_decomp_chain_scale_t::chksum_decomp_chain_scale_t(chksum_decomp_chain_scale_params_t params) :
    scale_test_name(__FUNCTION__),
    destructor_free_buffers(params.destructor_free_buffers_),
    success(false)
{
    chksum_decomp_chain_pre_push_params_t pre_push;
    dp_mem_t    *decomp_status_frag;
    dp_mem_t    *decomp_opaque_frag;
    dp_mem_t    *curr_chksum_status_host;
    dp_mem_t    *curr_chksum_opaque_host;
    uint32_t    max_hash_blks;
    uint32_t    max_opaque_blks;
    uint32_t    i;

    assert(params.num_chains_);
    for (i = 0; i < params.num_chains_; i++) {
        chksum_decomp_chain_vec.push_back(new chksum_decomp_chain_t(params.cdc_params_));
    }

    // Set up a common exp_opaque_data_buf that can be used to verify both
    // checksum opaque as well as decomp opaque.
    max_hash_blks = COMP_MAX_HASH_BLKS(params.cdc_params_.app_max_size_,
                                       kCompAppHashBlkSize);
    max_opaque_blks = std::max(max_hash_blks, params.num_chains_);
    exp_opaque_data_buf = new dp_mem_t(1, max_opaque_blks * sizeof(uint64_t),
                                       DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                       kMinHostMemAllocSize);
    exp_opaque_data = 0xddccbbaa;
    for (i = 0; i < max_opaque_blks; i++) {
        memcpy(exp_opaque_data_buf->read() + (i * sizeof(uint64_t)),
               &exp_opaque_data, sizeof(uint32_t));
    }
    exp_opaque_data_buf->write_thru();

    // Other inits
    decomp_status_host_buf = new dp_mem_t(params.num_chains_, CP_STATUS_PAD_ALIGNED_SIZE,
                                          DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                          kMinHostMemAllocSize);
    decomp_opaque_host_buf = new dp_mem_t(1, params.num_chains_ * sizeof(uint64_t),
                                         DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                         kMinHostMemAllocSize);
    for (i = 0; i < chksum_decomp_chain_vec.size(); i++) {
        decomp_status_host_buf->line_set(i);
        decomp_status_frag = decomp_status_host_buf->fragment_find(0, sizeof(cp_status_sha512_t));
        decomp_opaque_frag = decomp_opaque_host_buf->fragment_find(i * sizeof(uint64_t),
                                                               sizeof(uint64_t));

        // Hash is multi-block per chain and requires a vector of status vectors,
        // and a vector of opaque vectors!
        curr_chksum_status_host = new dp_mem_t(max_hash_blks, CP_STATUS_PAD_ALIGNED_SIZE,
                                              DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                              kMinHostMemAllocSize);
        curr_chksum_opaque_host = new dp_mem_t(max_hash_blks, sizeof(uint64_t),
                                              DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM,
                                              kMinHostMemAllocSize);
        chksum_decomp_chain_vec[i]->pre_push(pre_push.caller_chksum_status_vec(curr_chksum_status_host).
                                                      caller_chksum_opaque_vec(curr_chksum_opaque_host).
                                                      caller_decomp_status_buf(decomp_status_frag).
                                                      caller_decomp_opaque_buf(decomp_opaque_frag).
                                                      caller_opaque_data(exp_opaque_data));
        chksum_status_host_vec.push_back(curr_chksum_status_host);
        chksum_opaque_host_vec.push_back(curr_chksum_opaque_host);
    }
}


/*
 * Accelerator checksum to decompression chaining scale
 * Destructor
 */
chksum_decomp_chain_scale_t::~chksum_decomp_chain_scale_t()
{
    uint32_t    i;

    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.

    printf("%s success %u destructor_free_buffers %u\n",
           __FUNCTION__, success, destructor_free_buffers);
    for (i = 0; i < chksum_decomp_chain_vec.size(); i++) {
        delete chksum_decomp_chain_vec[i];
    }

    if (success && destructor_free_buffers) {

        for (i = 0; i < chksum_decomp_chain_vec.size(); i++) {
            delete chksum_status_host_vec[i];
            delete chksum_opaque_host_vec[i];
        }
        chksum_status_host_vec.clear();
        chksum_opaque_host_vec.clear();
        delete exp_opaque_data_buf;
        delete decomp_status_host_buf;
        delete decomp_opaque_host_buf;
    }
    chksum_decomp_chain_vec.clear();
}


/*
 * Accelerator checksum to decompression chaining scale
 * Establish initial push parameters
 */
void
chksum_decomp_chain_scale_t::push_params_set(chksum_decomp_chain_push_params_t params)
{
    push_params = params;
}


/*
 * Accelerator checksum to decompression chaining scale
 * Set up the test
 */
int
chksum_decomp_chain_scale_t::push(void)
{
    uint32_t    i;

    success = false;
    for (i = 0; i < chksum_decomp_chain_vec.size(); i++) {
        chksum_opaque_host_vec[i]->all_lines_clear_thru();

        /*
         * Note that chksum-decomp is not a true chain operation, i.e., both
         * operations can and will be initiated at the same time. See
         * explanations in chksum_decomp_chain_t::push().
         */
        chksum_decomp_chain_vec[i]->push(push_params);
    }
    return 0;
}


/*
 * Accelerator checksum to decompression chaining scale
 * Initiate the test
 */
int
chksum_decomp_chain_scale_t::post_push(void)
{
    uint32_t    num_submissions = chksum_decomp_chain_vec.size();

    /*
     * Only need to call post_push() once since all submissions went
     * to the same comp_ring.
     */
    assert(num_submissions);
    if (num_submissions) {
        chksum_decomp_chain_vec[num_submissions - 1]->post_push();
        run_count++;
    }
    return 0;
}


/*
 * Accelerator checksum to decompression chaining scale
 * Check for test completion and return 0 if so.
 *
 * Note: completion_check() must be non-blocking, i.e., do not use
 * tests::Poller within this function or any descendant functions.
 */
int
chksum_decomp_chain_scale_t::completion_check(void)
{
    int         actual_hash_blks;
    uint32_t    i;

    verification_time_advance();
    success = memcmp(decomp_opaque_host_buf->read_thru(),
                     exp_opaque_data_buf->read_thru(),
                     decomp_opaque_host_buf->line_size_get()) == 0;
    if (success) {
        actual_hash_blks = push_params.comp_hash_chain_->actual_hash_blks_get(
                                       TEST_RESOURCE_NON_BLOCKING_QUERY);
        success = (actual_hash_blks > 0);
    }
    for (i = 0; success && (i < chksum_decomp_chain_vec.size()); i++) {
        chksum_opaque_host_vec[i]->line_set(0);
        if (memcmp(chksum_opaque_host_vec[i]->read_thru(),
                   exp_opaque_data_buf->read_thru(),
                   actual_hash_blks * sizeof(uint64_t))) {
            success = false;
        }
    }

    return success ? 0 : -1;
}


/*
 * Accelerator checksum to decompression chaining scale
 */
int
chksum_decomp_chain_scale_t::fast_verify(void)
{
    uint32_t    i;

    success = true;
    for (i = 0; i < chksum_decomp_chain_vec.size(); i++) {
        if (chksum_decomp_chain_vec[i]->fast_verify()) {
            printf("ERROR: %s submission #%u failed\n",
                   __FUNCTION__, i);
            success = false;
        }
    }

    return success ? 0 : -1;
}

/*
 * Accelerator checksum to decompression chaining scale
 * Perform a slower but full verification of all chain results.
 */
int
chksum_decomp_chain_scale_t::full_verify(void)
{
    uint32_t    i;

    success = true;
    for (i = 0; i < chksum_decomp_chain_vec.size(); i++) {
        if (chksum_decomp_chain_vec[i]->full_verify()) {
            printf("ERROR: %s submission #%u failed\n",
                   __FUNCTION__, i);
            success = false;
        }
    }

    return success ? 0 : -1;
}


/*************************************************************************************/
/* Accelerator XTS encrypt only chaining scale                                       */
/*************************************************************************************/

/*
 * Accelerator XTS encrypt only chaining chaining scale
 * Constructor
 */
encrypt_only_scale_t::encrypt_only_scale_t(encrypt_only_scale_params_t params) :
    scale_test_name(__FUNCTION__),
    destructor_free_buffers(params.destructor_free_buffers_),
    success(false)
{
    encrypt_only_pre_push_params_t pre_push;
    dp_mem_t    *xts_status_frag;
    dp_mem_t    *xts_opaque_frag;
    uint32_t    i;

    // Instantiate unique encrypt_only_t tests one by one, as opposed to
    // using vector instantiation since it would use the same value for all
    // instances which is not what we want.
    assert(params.num_chains_);
    for (i = 0; i < params.num_chains_; i++) {
        encrypt_only_vec.push_back(new encrypt_only_t(params.enc_params_));
    }

    // Allocate status and opaque tags as a byte streams for fast memcmp.
    xts_status_host_buf = new dp_mem_t(1,  params.num_chains_ * sizeof(xts::xts_status_t),
                              DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM, sizeof(xts::xts_status_t));
    exp_status_data_buf = new dp_mem_t(1, params.num_chains_ * sizeof(xts::xts_status_t),
                                       DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
    xts_opaque_host_buf = new dp_mem_t(1,  params.num_chains_ * sizeof(uint64_t),
                              DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM, sizeof(uint64_t));
    exp_opaque_data_buf = new dp_mem_t(1, params.num_chains_ * sizeof(uint64_t),
                                       DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);

    exp_opaque_data = 0x7777777777777777;
    exp_opaque_data_buf->fill_thru(0x77);

    // Other inits

    for (i = 0; i < encrypt_only_vec.size(); i++) {
        xts_status_frag = xts_status_host_buf->fragment_find(i * sizeof(xts::xts_status_t),
                                                             sizeof(xts::xts_status_t));
        xts_opaque_frag = xts_opaque_host_buf->fragment_find(i * sizeof(uint64_t),
                                                             sizeof(uint64_t));
        encrypt_only_vec[i]->pre_push(pre_push.caller_xts_status_buf(xts_status_frag).
                                               caller_xts_opaque_buf(xts_opaque_frag).
                                               caller_xts_opaque_data(exp_opaque_data));
    }
}


/*
 * Accelerator XTS encrypt only chaining chaining scale
 * Destructor
 */
encrypt_only_scale_t::~encrypt_only_scale_t()
{
    uint32_t    i;

    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.

    printf("%s success %u destructor_free_buffers %u\n",
           __FUNCTION__, success, destructor_free_buffers);
    for (i = 0; i < encrypt_only_vec.size(); i++) {
        delete encrypt_only_vec[i];
    }
    encrypt_only_vec.clear();

    if (success && destructor_free_buffers) {

        delete xts_status_host_buf;
        delete xts_opaque_host_buf;
        delete exp_status_data_buf;
        delete exp_opaque_data_buf;
    }
}


/*
 * Accelerator XTS encrypt only chaining chaining scale
 * Establish initial push parameters
 */
void
encrypt_only_scale_t::push_params_set(encrypt_only_push_params_t params)
{
    push_params = params;
}


/*
 * Accelerator XTS encrypt only chaining chaining scale
 * Set up the test
 */
int
encrypt_only_scale_t::push(void)
{
    uint32_t    i;

    success = false;
    xts_opaque_host_buf->clear_thru();

    for (i = 0; i < encrypt_only_vec.size(); i++) {
        encrypt_only_vec[i]->push(push_params);
    }
    return 0;
}


/*
 * Accelerator XTS encrypt only chaining chaining scale
 * Initiate the test
 */
int
encrypt_only_scale_t::post_push(void)
{
    uint32_t    num_submissions = encrypt_only_vec.size();

    /*
     * Only need to call post_push() once since all submissions went
     * to the same XTS sequencer queue.
     */
    assert(num_submissions);
    if (num_submissions) {
        encrypt_only_vec[num_submissions - 1]->post_push();
        run_count++;
    }
    return 0;
}


/*
 * Accelerator XTS encrypt only chaining chaining scale
 * Check for test completion and return 0 if so.
 *
 * Note: completion_check() must be non-blocking, i.e., do not use
 * tests::Poller within this function or any descendant functions.
 */
int
encrypt_only_scale_t::completion_check(void)
{
    verification_time_advance();
    success = memcmp(xts_opaque_host_buf->read_thru(),
                     exp_opaque_data_buf->read_thru(),
                     xts_opaque_host_buf->line_size_get()) == 0;
    return success ? 0 : -1;
}


/*
 * Accelerator XTS encrypt only chaining chaining scale
 * Encryption is the last step in the chain so do a fast verification
 * of all XTS status entries.
 */
int
encrypt_only_scale_t::fast_verify(void)
{
    success = memcmp(xts_status_host_buf->read_thru(),
                     exp_status_data_buf->read_thru(),
                     xts_status_host_buf->line_size_get()) == 0;
    return success ? 0 : -1;
}

/*
 * Accelerator XTS encrypt only chaining chaining scale
 * Perform a slower but full verification of all chain results.
 */
int
encrypt_only_scale_t::full_verify(void)
{
    uint32_t    i;

    success = true;
    for (i = 0; i < encrypt_only_vec.size(); i++) {
        if (encrypt_only_vec[i]->full_verify()) {
            printf("ERROR: %s submission #%u failed\n",
                   __FUNCTION__, i);
            success = false;
        }
    }

    return success ? 0 : -1;
}


/*************************************************************************************/
/* Accelerator chaining scale test instantiations                                    */
/*************************************************************************************/

/*
 * Create and return an instance of
 *   Compression to XTS-encrypt chaining scale
 */
acc_scale_tests_t *
acc_comp_encrypt_chain_scale_create(uint32_t num_submissions,
                                    uint32_t app_blk_size,
                                    void *seed_test_chain,
                                    acc_scale_seq_sq_info_t& seq_sq_info)
{
    comp_encrypt_chain_scale_params_t   cec_scale;
    comp_encrypt_chain_params_t         cec_params;
    comp_encrypt_chain_push_params_t    cec_push;
    comp_encrypt_chain_scale_t          *comp_encrypt_chain_scale;

    comp_encrypt_chain_scale =
        new comp_encrypt_chain_scale_t(cec_scale.num_chains(num_submissions).
                                                 cec_params(cec_params.app_max_size(app_blk_size).
                                                                       app_enc_size(kCompAppHashBlkSize).
                                                                       uncomp_mem_type(DP_MEM_TYPE_HOST_MEM).
                                                                       comp_mem_type1(DP_MEM_TYPE_HBM).
                                                                       comp_mem_type2(DP_MEM_TYPE_HOST_MEM).
                                                                       comp_status_mem_type1(DP_MEM_TYPE_HBM).
                                                                       comp_status_mem_type2(DP_MEM_TYPE_HOST_MEM).
                                                                       encrypt_mem_type(DP_MEM_TYPE_HOST_MEM).
                                                                       destructor_free_buffers(true).
                                                                       suppress_info_log(true)).
                                                 destructor_free_buffers(true));
    comp_encrypt_chain_scale->push_params_set(cec_push.enc_dec_blk_type(XTS_ENC_DEC_PER_HASH_BLK).
                                                       app_blk_size(app_blk_size).
                                                       comp_ring(cp_ring).
                                                       push_type(ACC_RING_PUSH_SEQUENCER_BATCH).
                                                       seq_comp_qid(queues::get_seq_comp_sq(seq_sq_info.seq_sq_idx)).
                                                       seq_comp_status_qid(queues::get_seq_comp_status_sq(seq_sq_info.seq_status_sq_idx)));
    /*
     * Return number of sequencer queues actually consumed by the test
     */
    seq_sq_info.num_seq_sq_consumed = 1;
    seq_sq_info.num_seq_status_sq_consumed = num_submissions;
    return comp_encrypt_chain_scale;
}


/*
 * Create and return an instance of
 *   XTS-decrypt to decompression chaining scale
 */
acc_scale_tests_t *
acc_decrypt_decomp_chain_scale_create(uint32_t num_submissions,
                                      uint32_t app_blk_size,
                                      void *seed_test_chain,
                                      acc_scale_seq_sq_info_t& seq_sq_info)
{
    decrypt_decomp_chain_scale_params_t ddc_scale;
    decrypt_decomp_chain_params_t       ddc_params;
    decrypt_decomp_chain_push_params_t  ddc_push;
    decrypt_decomp_chain_scale_t        *decrypt_decomp_chain_scale;

    decrypt_decomp_chain_scale =
        new decrypt_decomp_chain_scale_t(ddc_scale.num_chains(num_submissions).
                                                   ddc_params(ddc_params.app_max_size(app_blk_size).
                                                                         app_enc_size(kCompAppHashBlkSize).
                                                                         uncomp_mem_type(DP_MEM_TYPE_HOST_MEM).
                                                                         xts_status_mem_type1(DP_MEM_TYPE_HBM).
                                                                         xts_status_mem_type2(DP_MEM_TYPE_HOST_MEM).
                                                                         decrypt_mem_type1(DP_MEM_TYPE_HBM).
                                                                         decrypt_mem_type2(DP_MEM_TYPE_VOID).
                                                                         destructor_free_buffers(true).
                                                                         suppress_info_log(true)).
                                                   destructor_free_buffers(true));
    decrypt_decomp_chain_scale->push_params_set(ddc_push.comp_encrypt_chain((comp_encrypt_chain_t *)seed_test_chain).
                                                         decomp_ring(dc_ring).
                                                         seq_xts_qid(queues::get_seq_xts_sq(seq_sq_info.seq_sq_idx)).
                                                         seq_xts_status_qid(queues::get_seq_xts_status_sq(seq_sq_info.seq_status_sq_idx)));
    /*
     * Return number of sequencer queues actually consumed by the test
     */
    seq_sq_info.num_seq_sq_consumed = 1;
    seq_sq_info.num_seq_status_sq_consumed = num_submissions;
    return decrypt_decomp_chain_scale;
}


/*
 * Create and return an instance of
 *   Compression to hash chaining scale
 */
acc_scale_tests_t *
acc_comp_hash_chain_scale_create(uint32_t num_submissions,
                                 uint32_t app_blk_size,
                                 void *seed_test_chain,
                                 acc_scale_seq_sq_info_t& seq_sq_info)
{
    comp_hash_chain_scale_params_t   chc_scale;
    comp_hash_chain_params_t         chc_params;
    comp_hash_chain_push_params_t    chc_push;
    comp_hash_chain_scale_t          *comp_hash_chain_scale;

    comp_hash_chain_scale =
        new comp_hash_chain_scale_t(chc_scale.num_chains(num_submissions).
                                              chc_params(chc_params.app_max_size(app_blk_size).
                                                                    uncomp_mem_type(DP_MEM_TYPE_HOST_MEM).
                                                                    comp_mem_type1(DP_MEM_TYPE_HBM).
                                                                    comp_mem_type2(DP_MEM_TYPE_HOST_MEM).
                                                                    comp_status_mem_type1(DP_MEM_TYPE_HBM).
                                                                    comp_status_mem_type2(DP_MEM_TYPE_HOST_MEM).
                                                                    destructor_free_buffers(true).
                                                                    suppress_info_log(true)).
                                              destructor_free_buffers(true));
    comp_hash_chain_scale->push_params_set(chc_push.app_blk_size(app_blk_size).
                                                    app_hash_size(kCompAppHashBlkSize).
                                                    integrity_type(COMP_INTEGRITY_M_ADLER32).
                                                    comp_ring(cp_ring).
                                                    hash_ring(cp_hot_ring).
                                                    push_type(ACC_RING_PUSH_SEQUENCER_BATCH).
                                                    seq_comp_qid(queues::get_seq_comp_sq(seq_sq_info.seq_sq_idx)).
                                                    seq_comp_status_qid(queues::get_seq_comp_status_sq(seq_sq_info.seq_status_sq_idx)));
    /*
     * Return number of sequencer queues actually consumed by the test
     */
    seq_sq_info.num_seq_sq_consumed = 1;
    seq_sq_info.num_seq_status_sq_consumed = num_submissions;
    return comp_hash_chain_scale;
}


/*
 * Create and return an instance of
 *   Checksum to decompression chaining scale
 */
acc_scale_tests_t *
acc_chksum_decomp_chain_scale_create(uint32_t num_submissions,
                                     uint32_t app_blk_size,
                                     void *seed_test_chain,
                                     acc_scale_seq_sq_info_t& seq_sq_info)
{
    chksum_decomp_chain_scale_params_t cdc_scale;
    chksum_decomp_chain_params_t       cdc_params;
    chksum_decomp_chain_push_params_t  cdc_push;
    chksum_decomp_chain_scale_t        *chksum_decomp_chain_scale;

    chksum_decomp_chain_scale =
        new chksum_decomp_chain_scale_t(cdc_scale.num_chains(num_submissions).
                                                  cdc_params(cdc_params.app_max_size(app_blk_size).
                                                                        uncomp_mem_type(DP_MEM_TYPE_HOST_MEM).
                                                                        destructor_free_buffers(true).
                                                                        suppress_info_log(true)).
                                                  destructor_free_buffers(true));
    chksum_decomp_chain_scale->push_params_set(cdc_push.comp_hash_chain((comp_hash_chain_t *)seed_test_chain).
                                                        chksum_ring(dc_ring).
                                                        decomp_ring(dc_hot_ring).
                                                        push_type(ACC_RING_PUSH_SEQUENCER_BATCH).
                                                        seq_chksum_qid(queues::get_seq_comp_sq(seq_sq_info.seq_sq_idx)).
                                                        seq_decomp_qid(queues::get_seq_comp_sq(seq_sq_info.seq_sq_idx + 1)));
    /*
     * Two sequencer queues are consumed above: 1 for chksum and 1 for decomp
     */
    seq_sq_info.num_seq_sq_consumed = 2;
    seq_sq_info.num_seq_status_sq_consumed = 0;
    return chksum_decomp_chain_scale;
}


/*
 * Create and return an instance of
 *   XTS-encrypt only chaining scale
 */
acc_scale_tests_t *
acc_encrypt_only_scale_create(uint32_t num_submissions,
                              uint32_t app_blk_size,
                              void *seed_test_chain,
                              acc_scale_seq_sq_info_t& seq_sq_info)
{
    encrypt_only_scale_params_t   enc_scale;
    encrypt_only_params_t         enc_params;
    encrypt_only_push_params_t    enc_push;
    encrypt_only_scale_t          *encrypt_only_scale;

    encrypt_only_scale =
        new encrypt_only_scale_t(enc_scale.num_chains(num_submissions).
                                           enc_params(enc_params.app_max_size(app_blk_size).
                                                                 unencrypt_mem_type(DP_MEM_TYPE_HOST_MEM).
                                                                 encrypt_mem_type(DP_MEM_TYPE_HOST_MEM).
                                                                 destructor_free_buffers(true).
                                                                 suppress_info_log(true)).
                                           destructor_free_buffers(true));
    encrypt_only_scale->push_params_set(enc_push.app_blk_size(app_blk_size).
                                                 push_type(ACC_RING_PUSH_SEQUENCER_BATCH).
                                                 seq_xts_qid(queues::get_seq_xts_sq(seq_sq_info.seq_sq_idx)));
    /*
     * Return number of sequencer queues actually consumed by the test
     */
    seq_sq_info.num_seq_sq_consumed = 1;
    seq_sq_info.num_seq_status_sq_consumed = 0;
    return encrypt_only_scale;
}


/*************************************************************************************/
/* Run Accelerator chaining scale test group                                         */
/*************************************************************************************/

/*
 * Run the accelerator scale test group based on the configured run_acc_scale_tests_map.
 */
int
acc_scale_tests_push(void)
{
	typedef std::pair<std::function<acc_scale_tests_t*(uint32_t,
                                                       uint32_t,
                                                       void*,
                                                       acc_scale_seq_sq_info_t&)>,
                      void*> acc_scale_tests_spec_t;

    std::vector<acc_scale_tests_t*>     seed_tests_vec;
    std::vector<acc_scale_tests_spec_t> tests_vec;

    comp_encrypt_chain_scale_t  *seed_comp_encrypt_chain_scale = nullptr;
    comp_encrypt_chain_t        *seed_comp_encrypt_chain = nullptr;
    comp_hash_chain_scale_t     *seed_comp_hash_chain_scale = nullptr;
    comp_hash_chain_t           *seed_comp_hash_chain = nullptr;
    acc_scale_seq_sq_info_t     acc_scale_seq_sq_info = {0};
    uint32_t                    num_submissions = NUM_TO_VAL(FLAGS_acc_scale_submissions);
    uint32_t                    num_replicas = NUM_TO_VAL(FLAGS_acc_scale_chain_replica);
    uint32_t                    app_blk_size = NUM_TO_VAL(FLAGS_acc_scale_blk_size);
    uint32_t                    poll_factor = app_blk_size / kCompAppMinSize;
    tests::Poller               poll(FLAGS_long_poll_interval * poll_factor);
    acc_scale_tests_list_t      tests_list(poll_factor);
    uint32_t                    r, v;
    int                         ret_val;

    /*
     * For the XTS-decrypt to decompression), generate seed data
     * by executing one Compression to XTS-encrypt chaining test.
     * Note that we'll do all the setup before starting any tests.
     */
    if (run_acc_scale_tests_map & ACC_SCALE_TEST_DECRYPT_DECOMP) {
        seed_comp_encrypt_chain_scale = (comp_encrypt_chain_scale_t *)
                                         acc_comp_encrypt_chain_scale_create(1, app_blk_size,
                                                          nullptr, acc_scale_seq_sq_info);
        seed_comp_encrypt_chain = seed_comp_encrypt_chain_scale->chain_get(0);
        seed_tests_vec.push_back(seed_comp_encrypt_chain_scale);
        tests_vec.push_back(std::make_pair(&acc_decrypt_decomp_chain_scale_create,
                                           (void *)seed_comp_encrypt_chain));
    }

    /*
     * Checksum-decompress also needs seed data
     */
    if (run_acc_scale_tests_map & ACC_SCALE_TEST_CHKSUM_DECOMP) {
        seed_comp_hash_chain_scale = (comp_hash_chain_scale_t *)
                                      acc_comp_hash_chain_scale_create(1, app_blk_size,
                                                    nullptr, acc_scale_seq_sq_info);
        seed_comp_hash_chain = seed_comp_hash_chain_scale->chain_get(0);
        seed_tests_vec.push_back(seed_comp_hash_chain_scale);
        tests_vec.push_back(std::make_pair(&acc_chksum_decomp_chain_scale_create,
                                           (void *)seed_comp_hash_chain));
    }

    /*
     * Add more tests as selected
     */
    if (run_acc_scale_tests_map & ACC_SCALE_TEST_COMP_ENCRYPT) {
        tests_vec.push_back(std::make_pair(&acc_comp_encrypt_chain_scale_create,
                                           nullptr));
    }
    if (run_acc_scale_tests_map & ACC_SCALE_TEST_COMP_HASH) {
        tests_vec.push_back(std::make_pair(&acc_comp_hash_chain_scale_create,
                                           nullptr));
    }
    if (run_acc_scale_tests_map & ACC_SCALE_TEST_ENCRYPT_ONLY) {
        tests_vec.push_back(std::make_pair(&acc_encrypt_only_scale_create,
                                           nullptr));
    }

    /*
     * Create test list,
     * each chain is replicated by num_replicas times
     */
    acc_scale_seq_sq_info.seq_sq_idx = 0;
    acc_scale_seq_sq_info.seq_status_sq_idx = 0;
    for (r = 0; r < num_replicas; r++) {
        for (v = 0; v < tests_vec.size(); v++) {
            tests_list.push(tests_vec[v].first(num_submissions, app_blk_size,
                                               tests_vec[v].second,
                                               acc_scale_seq_sq_info));
            acc_scale_seq_sq_info.seq_sq_idx +=
                                  acc_scale_seq_sq_info.num_seq_sq_consumed;
            acc_scale_seq_sq_info.seq_status_sq_idx +=
                                  acc_scale_seq_sq_info.num_seq_status_sq_consumed;
        }
    }

    /*
     * Start any seed tests
     */
    for (v = 0; v < seed_tests_vec.size(); v++) {
        acc_scale_tests_t *seed_test_chain = seed_tests_vec[v];

        seed_test_chain->push();
        seed_test_chain->post_push();

        /*
         * Poll for completion of seed data generation
         */
        auto completion_poll = [&seed_test_chain] () -> int
        {
            return seed_test_chain->completion_check();
        };

        /*
         * Seed data must use full_verify() to ensure validity and
         * seed state updates such as cp_output_data_len, etc.
         */
        if (poll(completion_poll) ||
            seed_test_chain->full_verify()) {

            printf("Seed %s data creation failed\n",
                   seed_test_chain->scale_test_name_get());
            return -1;
        }
    }


    /*
     * Now start all the configured scaled tests
     */
    ret_val = tests_list.post_push(__FUNCTION__);
    if (ret_val == 0) {
        if (seed_comp_encrypt_chain_scale) {
            delete seed_comp_encrypt_chain_scale;
        }
        if (seed_comp_hash_chain_scale) {
            delete seed_comp_hash_chain_scale;
        }
    }
    return ret_val;
}


/*
 * List of accelerator scale tests
 * Constructor
 */
acc_scale_tests_list_t::acc_scale_tests_list_t(uint32_t poll_factor) :
    poll_factor(poll_factor)
{
}


/*
 * List of accelerator scale tests
 * Destructor
 */
acc_scale_tests_list_t::~acc_scale_tests_list_t()
{
    acc_scale_tests_t   *scale_test;
    std::list<acc_scale_tests_t*>::iterator it;

    printf("%s\n", __FUNCTION__);
    for (it = compl_list.begin(); it != compl_list.end(); it++) {
        scale_test = *it;
        delete scale_test;
    }

    for (it = tests_list.begin(); it != tests_list.end(); it++) {
        scale_test = *it;
        delete scale_test;
    }

    compl_list.clear();
    tests_list.clear();
}


/*
 * List of accelerator scale tests
 * Add a test to list
 */
void
acc_scale_tests_list_t::push(acc_scale_tests_t *scale_test)
{
    tests_list.push_back(scale_test);
}


/*
 * List of accelerator scale tests
 * Execute the test list for the duration configured by FLAGS_acc_scale_iters.
 */
int
acc_scale_tests_list_t::post_push(const char *test_name)
{
    tests::Poller       poll(FLAGS_long_poll_interval * poll_factor);
    acc_scale_tests_t   *scale_test;
    std::list<acc_scale_tests_t*>::iterator it;
    int                 verify_result = 0;

    /*
     * Start the tests:
     *
     * Note: to facilitate overlapped operations in RTL, we inititate the
     * tests in 2 steps: 1- push, which sets up the tests (i.e., descriptor
     * writes, etc.), and 2- post_push which rings doorbells.
     */
    for (it = tests_list.begin(); it != tests_list.end(); it++) {
        scale_test = *it;
        scale_test->push();
    }
    for (it = tests_list.begin(); it != tests_list.end(); it++) {
        scale_test = *it;
        scale_test->post_push();
    }

    /*
     * Resubmit qualified tests that have completed
     */
    auto qualified_resubmit = [this, &verify_result] () -> void
    {
        std::list<acc_scale_tests_t*>::iterator it;
        acc_scale_tests_t   *scale_test;

        // Restart any tests that have not exhausted iteration limit
        it = compl_list.begin();
        while (it != compl_list.end()) {

            // Ensure fast/full verification also passes
            scale_test = *it;
            verify_result = FLAGS_with_rtl_skipverify ||
                            (FLAGS_acc_scale_verify_method == "fast") ?
                            scale_test->fast_verify() : scale_test->full_verify();
            if (verify_result) {
                break;
            }
            if (FLAGS_acc_scale_iters &&
                (scale_test->run_count_get() >= FLAGS_acc_scale_iters)) {
                delete scale_test;

            } else {

                // restart the current test
                push(scale_test);
                scale_test->push();
                scale_test->post_push();
            }
            it = compl_list.erase(it);
        }
    };

    /*
     * Poll for completion of tests in the list while resubmitting those
     * that have completed.
     */
    auto completion_poll = [this, qualified_resubmit] () -> int
    {
        uint32_t    compl_count;

        assert(compl_list.size() == 0);

        completion_check();
        compl_count = compl_list.size();
        qualified_resubmit();

        /*
         * Return completion done if at least one test completed
         */
        return compl_count ? 0 : -1;
    };

    while (!tests_list.empty()) {
        if (poll(completion_poll)) {
            printf("test %s timed out\n", test_name);
            break;
        }

        if (verify_result) {
            break;
        }
        usleep(10000);
    }

    outstanding_report();
    return (verify_result == 0) && tests_list.empty() ? 0 : -1;
}


/*
 * List of accelerator scale tests
 * Check for test list completion and return 0 if so
 */
int
acc_scale_tests_list_t::completion_check(void)
{
    acc_scale_tests_t   *scale_test;
    std::list<acc_scale_tests_t*>::iterator it;

    it = tests_list.begin();
    while (it != tests_list.end()) {
        scale_test = *it;

        if (scale_test->completion_check() == 0) {
            printf("Completed iteration %lu of %s\n",
                   scale_test->run_count_get(),
                   scale_test->scale_test_name_get());

            compl_list.push_back(scale_test);
            it = tests_list.erase(it);

        } else {
            it++;
        }
    }

    return tests_list.empty() ? 0 : -1;
}


/*
 * Do full status verification report of all outstanding tests
 */
void
acc_scale_tests_list_t::outstanding_report(void)
{
    acc_scale_tests_t   *scale_test;
    std::list<acc_scale_tests_t*>::iterator it;

    for (it = tests_list.begin(); it != tests_list.end(); it++) {
        scale_test = *it;

        printf("\nOutstanding test report for %s:\n",
               scale_test->scale_test_name_get());
        scale_test->full_verify();
    }
}


/*
 * Validate application block size
 */
bool
acc_scale_tests_blk_size_validate(const char *flag_name,
                                  uint64_t value)
{
    uint64_t    blk_size = NUM_TO_VAL(value);

    if ((blk_size >= kCompAppMinSize) &&
        (blk_size <= kCompEngineMaxSize)) {
        return true;
    }

    printf("Value for --%s (in power of 2) must be >= %d and <= %d\n",
           flag_name, (int)log2(kCompAppMinSize), (int)log2(kCompAppMaxSize));
    return false;
}

}  // namespace tests
