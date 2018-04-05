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
    dp_mem_t    *xts_status_frag;
    dp_mem_t    *xts_opaque_frag;
    uint32_t    i;

    // Instantiate unique comp_encrypt_chain_t tests one by one, as opposed to
    // using vector instantiation since it would use the same value for all 
    // instances which is not what we want.
    assert(params.num_chains_);
    for (i = 0; i < params.num_chains_; i++) {
        comp_encrypt_chain_vec.push_back(new comp_encrypt_chain_t(params.cec_params_));
    }

    // Allocate status and opaque tags as a byte streams for fast memcmp
    xts_status_host_buf = new dp_mem_t(1,  params.num_chains_ * sizeof(uint32_t),
                              DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM, sizeof(uint32_t));
    exp_status_data_buf = new dp_mem_t(1, params.num_chains_ * sizeof(uint32_t),
                                       DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);
    xts_opaque_host_buf = new dp_mem_t(1,  params.num_chains_ * sizeof(uint64_t),
                              DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM, sizeof(uint64_t));
    exp_opaque_data_buf = new dp_mem_t(1, params.num_chains_ * sizeof(uint64_t),
                                       DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);

    exp_opaque_data = 0xcccccccccccccccc;
    exp_opaque_data_buf->fill_thru(0xcc);

    // Other inits
    comp_pad_buf = new dp_mem_t(1, 4096, DP_MEM_ALIGN_PAGE);

    for (i = 0; i < comp_encrypt_chain_vec.size(); i++) {
        xts_status_frag = xts_status_host_buf->fragment_find(i * sizeof(uint32_t),
                                                             sizeof(uint32_t));
        xts_opaque_frag = xts_opaque_host_buf->fragment_find(i * sizeof(uint64_t),
                                                             sizeof(uint64_t));
        comp_encrypt_chain_vec[i]->pre_push(pre_push.caller_comp_pad_buf(comp_pad_buf).
                                                     caller_xts_status_buf(xts_status_frag).
                                                     caller_xts_opaque_buf(xts_opaque_frag).
                                                     caller_xts_opaque_data(exp_opaque_data));
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
    comp_encrypt_chain_vec.clear();

    if (success && destructor_free_buffers) {

        delete xts_status_host_buf;
        delete xts_opaque_host_buf;
        delete exp_status_data_buf;
        delete exp_opaque_data_buf;
        delete comp_pad_buf;
    }
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
 * Initiate the test
 */
int 
comp_encrypt_chain_scale_t::push(void)
{
    uint32_t    i;

    success = false;
    xts_opaque_host_buf->clear_thru();

    for (i = 0; i < comp_encrypt_chain_vec.size(); i++) {

        // Push all entries on the same compression SQ but each submission 
        // must be handled with its own comp status SQ since the completion
        // order will not be known.
        push_params.seq_comp_status_qid_ = queues::get_seq_comp_status_sq(i);

        // Encryption is last in chain so does not require status sequencer
        push_params.seq_xts_status_qid_ = 0;

        comp_encrypt_chain_vec[i]->push(push_params);
    }

    // Only need to call post_push() once since all submissions went
    // to the same comp_queue.
    comp_encrypt_chain_vec[i - 1]->post_push();

    run_count++;
    return 0;
}


/*
 * Accelerator compression to XTS-encrypt chaining scale
 * Check for test completion and return 0 if so.
 */
int 
comp_encrypt_chain_scale_t::completion_check(void)
{
    if (memcmp(xts_opaque_host_buf->read_thru(),
               exp_opaque_data_buf->read_thru(),
               xts_opaque_host_buf->line_size_get()) == 0) {
        success = true;
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
    if (memcmp(xts_status_host_buf->read_thru(),
               exp_status_data_buf->read_thru(),
               xts_status_host_buf->line_size_get()) == 0) {
        success = true;
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

    for (i = 0; i < comp_encrypt_chain_vec.size(); i++) {
        if (comp_encrypt_chain_vec[i]->verify()) {
            printf("ERROR: %s submission #%u failed\n",
                   __FUNCTION__, i);
            success = false;
        }
    }

    return success ? 0 : -1;
}


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
    comp_opaque_host_buf = new dp_mem_t(1,  params.num_chains_ * sizeof(uint32_t),
                               DP_MEM_ALIGN_SPEC, DP_MEM_TYPE_HOST_MEM, sizeof(uint32_t));
    exp_opaque_data_buf = new dp_mem_t(1, params.num_chains_ * sizeof(uint32_t),
                                       DP_MEM_ALIGN_NONE, DP_MEM_TYPE_HOST_MEM);

    exp_opaque_data = 0xdddddddd;
    exp_opaque_data_buf->fill_thru(0xdd);

    // Other inits

    for (i = 0; i < decrypt_decomp_chain_vec.size(); i++) {
        comp_status_host_buf->line_set(i);
        comp_status_frag = comp_status_host_buf->fragment_find(0, sizeof(cp_status_sha512_t));
        comp_opaque_frag = comp_opaque_host_buf->fragment_find(i * sizeof(uint32_t),
                                                               sizeof(uint32_t));
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
 * Initiate the test
 */
int 
decrypt_decomp_chain_scale_t::push(void)
{
    uint32_t    i;

    success = false;
    comp_opaque_host_buf->clear_thru();

    for (i = 0; i < decrypt_decomp_chain_vec.size(); i++) {

        // Push all entries on the same compression SQ but each submission 
        // must be handled with its own XTS status SQ since the completion
        // order will not be known.
        push_params.seq_xts_status_qid_ = queues::get_seq_xts_status_sq(i);

        // Decompression is last in chain so does not require status sequencer
        push_params.seq_comp_status_qid_ = 0;

        decrypt_decomp_chain_vec[i]->push(push_params);
    }

    // Only need to call post_push() once since all submissions went
    // to the same XTS sequencer queue.
    decrypt_decomp_chain_vec[i - 1]->post_push();

    run_count++;
    return 0;
}


/*
 * Accelerator XTS-decrypt to decompression chaining scale
 * Check for test completion and return 0 if so.
 */
int 
decrypt_decomp_chain_scale_t::completion_check(void)
{
    if (memcmp(comp_opaque_host_buf->read_thru(),
               exp_opaque_data_buf->read_thru(),
               comp_opaque_host_buf->line_size_get()) == 0) {
        success = true;
    }

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
    cp_status_sha512_t  *st;
    uint32_t            i;

    for (i = 0; i < decrypt_decomp_chain_vec.size(); i++) {
        comp_status_host_buf->line_set(i);
        st = (cp_status_sha512_t *)comp_status_host_buf->read_thru();

        if (!st->valid) {
            printf("%s submission #%u: Status valid bit not set\n",
                   __FUNCTION__, i);
            success = false;
        }
        if (st->err) {
            printf("%s submission #%u status err 0x%x is unexpected\n",
                   __FUNCTION__, i, st->err);
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

    for (i = 0; i < decrypt_decomp_chain_vec.size(); i++) {
        if (decrypt_decomp_chain_vec[i]->verify()) {
            printf("ERROR: %s submission #%u failed\n",
                   __FUNCTION__, i);
            success = false;
        }
    }

    return success ? 0 : -1;
}


/*
 * Create and return an instance of 
 *   Compression to XTS-encrypt chaining scale
 */
acc_scale_tests_t * 
acc_comp_encrypt_chain_scale_create(void)
{
    comp_encrypt_chain_scale_params_t   cec_scale;
    comp_encrypt_chain_params_t         cec_params;
    comp_encrypt_chain_push_params_t    cec_push;
    comp_encrypt_chain_scale_t          *comp_encrypt_chain_scale;
    uint32_t                            app_blk_size = NUM_TO_VAL(FLAGS_acc_scale_blk_size);
    uint32_t                            num_submissions = NUM_TO_VAL(FLAGS_acc_scale_submissions);

    /*
     * Submit one less than queue size potential to ensure CI != PI
     */
    if (num_submissions > 1) {
        num_submissions--;
    }
    comp_encrypt_chain_scale = 
        new comp_encrypt_chain_scale_t(cec_scale.num_chains(num_submissions).
                                                 cec_params(cec_params.app_max_size(app_blk_size).
                                                                       uncomp_mem_type(DP_MEM_TYPE_HOST_MEM).
                                                                       comp_mem_type(DP_MEM_TYPE_HBM).
                                                                       comp_status_mem_type1(DP_MEM_TYPE_HBM).
                                                                       comp_status_mem_type2(DP_MEM_TYPE_HOST_MEM).
                                                                       encrypt_mem_type(DP_MEM_TYPE_HOST_MEM).
                                                                       destructor_free_buffers(true).
                                                                       suppress_info_log(true)).
                                                 destructor_free_buffers(true));
    comp_encrypt_chain_scale->push_params_set(cec_push.app_blk_size(app_blk_size).
                                                       comp_queue(cp_queue).
                                                       push_type(COMP_QUEUE_PUSH_SEQUENCER_DEFER).
                                                       seq_comp_qid(queues::get_seq_comp_sq(0)).
                                                       seq_comp_status_qid(queues::get_seq_comp_status_sq(0)));
    return comp_encrypt_chain_scale;
}


/*
 * Create and return an instance of 
 *   XTS-decrypt to decompression chaining scale
 */
acc_scale_tests_t * 
acc_decrypt_decomp_chain_scale_create(void)
{
    decrypt_decomp_chain_scale_params_t ddc_scale;
    decrypt_decomp_chain_params_t       ddc_params;
    decrypt_decomp_chain_push_params_t  ddc_push;
    decrypt_decomp_chain_scale_t        *decrypt_decomp_chain_scale;
    uint32_t                            app_blk_size = NUM_TO_VAL(FLAGS_acc_scale_blk_size);
    uint32_t                            num_submissions = NUM_TO_VAL(FLAGS_acc_scale_submissions);
    int                                 ret_val = -1;

    /*
     * First, generate source data by executing one Compression to XTS-encrypt
     * chaining test.
     */
    if (app_blk_size == kCompAppMinSize) {
        ret_val = seq_compress_output_encrypt_app_min_size();
    } else if (app_blk_size == kCompAppMaxSize) {
        ret_val = seq_compress_output_encrypt_app_max_size();
    } else if (app_blk_size == kCompAppNominalSize) {
        ret_val = seq_compress_output_encrypt_app_nominal_size();
    }
    if (ret_val) {
        printf("%s source data creation failed\n", __FUNCTION__);
    }

    /*
     * Submit one less than queue size potential to ensure CI != PI
     */
    if (num_submissions > 1) {
        num_submissions--;
    }
    decrypt_decomp_chain_scale = 
        new decrypt_decomp_chain_scale_t(ddc_scale.num_chains(num_submissions).
                                                   ddc_params(ddc_params.app_max_size(app_blk_size).
                                                                         uncomp_mem_type(DP_MEM_TYPE_HOST_MEM).
                                                                         xts_status_mem_type1(DP_MEM_TYPE_HBM).
                                                                         xts_status_mem_type2(DP_MEM_TYPE_HOST_MEM).
                                                                         decrypt_mem_type(DP_MEM_TYPE_HBM).
                                                                         destructor_free_buffers(true).
                                                                         suppress_info_log(true)).
                                                   destructor_free_buffers(true));
    decrypt_decomp_chain_scale->push_params_set(ddc_push.comp_encrypt_chain(comp_encrypt_chain).
                                                         decomp_queue(dc_queue).
                                                         seq_xts_qid(queues::get_seq_xts_sq(0)).
                                                         seq_xts_status_qid(queues::get_seq_xts_status_sq(0)));
    return decrypt_decomp_chain_scale;
}


/*
 * Run the accelerator scale test group which includes
 *   Compression to XTS-encrypt chaining scale
 *   XTS-decrypt to decompression chaining scale
 */
int
acc_scale_tests_comp_encrypt_decrypt_decomp(void)
{
    std::vector<std::function<acc_scale_tests_t*(void)>> tests_vec = { 
        &acc_comp_encrypt_chain_scale_create,
        &acc_decrypt_decomp_chain_scale_create,
    };

    acc_scale_tests_list_t  tests_list;

    /*
     * Create and run test list
     */
    for (uint32_t i = 0; i < tests_vec.size(); i++) {
        tests_list.add(tests_vec[i]());
    }

    return tests_list.run(__FUNCTION__);
}


/*
 * List of accelerator scale tests
 * Constructor
 */
acc_scale_tests_list_t::acc_scale_tests_list_t() :
    num_tests_completed(0),
    num_tests_failed(0)
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

    tests_list.clear();
}


/*
 * List of accelerator scale tests
 * Add a test to list
 */
void 
acc_scale_tests_list_t::add(acc_scale_tests_t *scale_test)
{
    tests_list.push_back(scale_test);
}


/*
 * List of accelerator scale tests
 * Execute the test list for the duration configured by FLAGS_acc_scale_iters.
 */
int
acc_scale_tests_list_t::run(const char *test_name)
{
    tests::Poller       poll(FLAGS_long_poll_interval);
    acc_scale_tests_t   *scale_test;
    std::list<acc_scale_tests_t*>::iterator it;
    int                 verify_result;

    /*
     * Start the tests
     */
    for (it = tests_list.begin(); it != tests_list.end(); it++) {
        scale_test = *it;
        scale_test->push();
    }

    /*
     * Poll for completion of all tests in the list
     */
    auto completion_poll = [this] () -> int
    {
        return completion_check();
    };

    while ((num_tests_completed + num_tests_failed) < tests_list.size()) {

        if (poll(completion_poll)) {
            printf("test %s timed out\n", test_name);
            break;
        }

        // Restart any tests that have not exhausted iteration limit
        it = compl_list.begin(); 
        while (it != compl_list.end()) {

            // Ensure fast/full verification also passes
            scale_test = *it;
            verify_result = FLAGS_acc_scale_verify_method == "fast" ?
                            scale_test->fast_verify() : scale_test->full_verify();
            if (verify_result) {
                break;
            }
            if (FLAGS_acc_scale_iters &&
                (scale_test->run_count_get() >= FLAGS_acc_scale_iters)) {
                delete scale_test;

            } else {

                // restart the current test
                add(scale_test);
                scale_test->push();
            }
            it = compl_list.erase(it);
        }
    }

    outstanding_report();
    return num_tests_completed == tests_list.size() ? 0 : -1;
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
    uint32_t            num_tests_checked = 0;

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

    return num_tests_checked == tests_list.size() ? 0 : -1;
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

        if (!scale_test->run_completed_get()) {
            printf("\nOutstanding test report for %s:\n",
                   scale_test->scale_test_name_get());
            scale_test->full_verify();
        }
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

    if ((blk_size == kCompAppMaxSize)   ||
        (blk_size == kCompAppMinSize)   ||
        (blk_size == kCompAppNominalSize)) {

        return true;
    }

    printf("Value for --%s (in power of 2) must be >= %d and <= %d\n",
           flag_name, (int)log2(kCompAppMinSize), (int)log2(kCompAppMaxSize));
    return false;
}

}  // namespace tests
