#ifndef	ACC_SCALE_TESTS_HPP_
#define	ACC_SCALE_TESTS_HPP_

#include <vector>
#include <list>
#include "decrypt_decomp_chain.hpp"
#include "comp_encrypt_chain.hpp"
#include "comp_hash_chain.hpp"
#include "chksum_decomp_chain.hpp"

/*
 * Accelerator scale tests DOL
 */
namespace tests {

/*
 * Base class
 */
class acc_scale_tests_t
{
public:

    acc_scale_tests_t() :
        run_count(0)
    {
        /*
         * Allocate a scratch buffer for use by verification_time_advance().
         */
        scratch_buf = new dp_mem_t(1, sizeof(uint64_t));
    }

    virtual ~acc_scale_tests_t()
    {
        delete scratch_buf;
    }

    // Pure virtual functions.
    virtual int push(void) = 0;
    virtual int completion_check(void) = 0;
    virtual int fast_verify(void) = 0;
    virtual int full_verify(void) = 0;
    virtual const char *scale_test_name_get(void) = 0;

    virtual uint64_t run_count_get(void)
    {
        return run_count;
    }

    /*
     * Advance verification time:
     *
     * This function is intended for support of the RTL --skipverify option.
     * When --skipverify is in effect, all calls to step_doorbell() become
     * non-blocking, allowing many doorbell events to be submitted concurrently
     * before the RTL actually starts them. This is a desired feature for scaled/stress
     * testing. There are 2 ways to trigger the RTL start:
     *   1) Caller invokes exit_simulation() which would then start all the events
     *      and ensure they all finish. However, this would also end the simulation
     *      upon completion which is not what we want from a stress test perspective.
     *   2) Some sort of HBM access occurs, which would advance verification time.
     *      Note: polling on host memory does not advance time.
     *
     * This function provides non-blocking poll on an HBM location in use by the test to
     * ensure that simulation time advances.
     */
    virtual void verification_time_advance(void)
    {
        assert(scratch_buf->is_mem_type_hbm());
        scratch_buf->read_thru();
    }

protected:
    dp_mem_t        *scratch_buf;
    uint64_t        run_count;
};


/*
 * Emulate named parameters support for comp_encrypt_chain_scale_t constructor
 */
class comp_encrypt_chain_scale_params_t
{
public:

    comp_encrypt_chain_scale_params_t() :
        num_chains_(0),
        destructor_free_buffers_(false)
    {
    }

    uint32_t                    num_chains_;
    comp_encrypt_chain_params_t cec_params_;
    bool                        destructor_free_buffers_;

    comp_encrypt_chain_scale_params_t&
    num_chains(uint32_t num_chains)
    {
        num_chains_ = num_chains;
        return *this;
    }
    comp_encrypt_chain_scale_params_t&
    cec_params(comp_encrypt_chain_params_t cec_params)
    {
        cec_params_ = cec_params;
        return *this;
    }
    comp_encrypt_chain_scale_params_t&
    destructor_free_buffers(bool destructor_free_buffers)
    {
        destructor_free_buffers_ = destructor_free_buffers;
        return *this;
    }
};


/*
 * Accelerator compression to XTS-encrypt chaining scale
 */
class comp_encrypt_chain_scale_t : public acc_scale_tests_t
{
public:
    comp_encrypt_chain_scale_t(comp_encrypt_chain_scale_params_t params);
    ~comp_encrypt_chain_scale_t();

    void push_params_set(comp_encrypt_chain_push_params_t params);

    virtual int push(void);
    virtual int completion_check(void);
    virtual int fast_verify(void);
    virtual int full_verify(void);

    virtual const char *scale_test_name_get(void)
    {
        return scale_test_name;
    }

    // Return a test instance from comp_encrypt_chain_vec
    comp_encrypt_chain_t *chain_get(uint32_t inst)
    {
        return inst < comp_encrypt_chain_vec.size() ?
               comp_encrypt_chain_vec[inst] : nullptr;
    }

private:
    std::vector<comp_encrypt_chain_t*>  comp_encrypt_chain_vec;

    const char      *scale_test_name;
    comp_encrypt_chain_push_params_t    push_params;

    dp_mem_t        *comp_pad_buf;
    dp_mem_t        *xts_status_host_buf;
    dp_mem_t        *xts_opaque_host_buf;

    dp_mem_t        *exp_status_data_buf;
    dp_mem_t        *exp_opaque_data_buf;
    uint64_t        exp_opaque_data;

    bool            destructor_free_buffers;
    bool            success;
};


/*
 * Emulate named parameters support for comp_hash_chain_scale_t constructor
 */
class comp_hash_chain_scale_params_t
{
public:

    comp_hash_chain_scale_params_t() :
        num_chains_(0),
        destructor_free_buffers_(false)
    {
    }

    uint32_t                    num_chains_;
    comp_hash_chain_params_t    chc_params_;
    bool                        destructor_free_buffers_;

    comp_hash_chain_scale_params_t&
    num_chains(uint32_t num_chains)
    {
        num_chains_ = num_chains;
        return *this;
    }
    comp_hash_chain_scale_params_t&
    chc_params(comp_hash_chain_params_t chc_params)
    {
        chc_params_ = chc_params;
        return *this;
    }
    comp_hash_chain_scale_params_t&
    destructor_free_buffers(bool destructor_free_buffers)
    {
        destructor_free_buffers_ = destructor_free_buffers;
        return *this;
    }
};


/*
 * Accelerator compression to hash chaining scale
 */
class comp_hash_chain_scale_t : public acc_scale_tests_t
{
public:
    comp_hash_chain_scale_t(comp_hash_chain_scale_params_t params);
    ~comp_hash_chain_scale_t();

    void push_params_set(comp_hash_chain_push_params_t params);

    virtual int push(void);
    virtual int completion_check(void);
    virtual int fast_verify(void);
    virtual int full_verify(void);

    virtual const char *scale_test_name_get(void)
    {
        return scale_test_name;
    }

private:
    std::vector<comp_hash_chain_t*>  comp_hash_chain_vec;

    const char      *scale_test_name;
    comp_hash_chain_push_params_t    push_params;

    dp_mem_t        *comp_pad_buf;

    std::vector<dp_mem_t*> hash_status_host_vec;
    std::vector<dp_mem_t*> hash_opaque_host_vec;

    dp_mem_t        *exp_opaque_data_buf;
    uint32_t        exp_opaque_data;

    bool            destructor_free_buffers;
    bool            success;
};


/*
 * Emulate named parameters support for chksum_decomp_chain_scale_t constructor
 */
class chksum_decomp_chain_scale_params_t
{
public:

    chksum_decomp_chain_scale_params_t() :
        num_chains_(0),
        actual_hash_blks_(0),
        destructor_free_buffers_(false)
    {
    }

    uint32_t                        num_chains_;
    uint32_t                        actual_hash_blks_;
    chksum_decomp_chain_params_t    cdc_params_;
    bool                            destructor_free_buffers_;

    chksum_decomp_chain_scale_params_t&
    num_chains(uint32_t num_chains)
    {
        num_chains_ = num_chains;
        return *this;
    }
    chksum_decomp_chain_scale_params_t&
    actual_hash_blks(uint32_t actual_hash_blks)
    {
        actual_hash_blks_ = actual_hash_blks;
        return *this;
    }
    chksum_decomp_chain_scale_params_t&
    cdc_params(chksum_decomp_chain_params_t cdc_params)
    {
        cdc_params_ = cdc_params;
        return *this;
    }
    chksum_decomp_chain_scale_params_t&
    destructor_free_buffers(bool destructor_free_buffers)
    {
        destructor_free_buffers_ = destructor_free_buffers;
        return *this;
    }
};


/*
 * Accelerator checksum to decompression chaining scale
 */
class chksum_decomp_chain_scale_t : public acc_scale_tests_t
{
public:
    chksum_decomp_chain_scale_t(chksum_decomp_chain_scale_params_t params);
    ~chksum_decomp_chain_scale_t();

    void push_params_set(chksum_decomp_chain_push_params_t params);

    virtual int push(void);
    virtual int completion_check(void);
    virtual int fast_verify(void);
    virtual int full_verify(void);

    virtual const char *scale_test_name_get(void)
    {
        return scale_test_name;
    }

private:
    std::vector<chksum_decomp_chain_t*>  chksum_decomp_chain_vec;

    const char      *scale_test_name;
    chksum_decomp_chain_push_params_t    push_params;

    std::vector<dp_mem_t*> chksum_status_host_vec;
    std::vector<dp_mem_t*> chksum_opaque_host_vec;

    dp_mem_t        *decomp_status_host_buf;
    dp_mem_t        *decomp_opaque_host_buf;
    dp_mem_t        *exp_opaque_data_buf;
    uint32_t        exp_opaque_data;
    uint32_t        actual_hash_blks;

    bool            destructor_free_buffers;
    bool            success;
};


/*
 * Emulate named parameters support for decrypt_decomp_chain_scale_t constructor
 */
class decrypt_decomp_chain_scale_params_t
{
public:

    decrypt_decomp_chain_scale_params_t() :
        num_chains_(0),
        destructor_free_buffers_(false)
    {
    }

    uint32_t                        num_chains_;
    decrypt_decomp_chain_params_t   ddc_params_;
    bool                            destructor_free_buffers_;

    decrypt_decomp_chain_scale_params_t&
    num_chains(uint32_t num_chains)
    {
        num_chains_ = num_chains;
        return *this;
    }
    decrypt_decomp_chain_scale_params_t&
    ddc_params(decrypt_decomp_chain_params_t ddc_params)
    {
        ddc_params_ = ddc_params;
        return *this;
    }
    decrypt_decomp_chain_scale_params_t&
    destructor_free_buffers(bool destructor_free_buffers)
    {
        destructor_free_buffers_ = destructor_free_buffers;
        return *this;
    }
};


/*
 * Accelerator XTS-decrypt to decompression chaining scale
 */
class decrypt_decomp_chain_scale_t : public acc_scale_tests_t
{
public:
    decrypt_decomp_chain_scale_t(decrypt_decomp_chain_scale_params_t params);
    ~decrypt_decomp_chain_scale_t();

    void push_params_set(decrypt_decomp_chain_push_params_t params);

    virtual int push(void);
    virtual int completion_check(void);
    virtual int fast_verify(void);
    virtual int full_verify(void);

    virtual const char *scale_test_name_get(void)
    {
        return scale_test_name;
    }

private:
    std::vector<decrypt_decomp_chain_t*> decrypt_decomp_chain_vec;

    const char          *scale_test_name;
    decrypt_decomp_chain_push_params_t  push_params;

    dp_mem_t            *comp_status_host_buf;
    dp_mem_t            *comp_opaque_host_buf;

    dp_mem_t            *exp_opaque_data_buf;
    uint32_t            exp_opaque_data;

    bool                destructor_free_buffers;
    bool                success;
};


/*
 * List of accelerator scale tests
 */
class acc_scale_tests_list_t
{
public:

    acc_scale_tests_list_t();
    ~acc_scale_tests_list_t();

    void push(acc_scale_tests_t *scale_test);
    int post_push(const char *test_name);
    int completion_check(void);
    void outstanding_report(void);

private:
    std::list<acc_scale_tests_t*>   tests_list;
    std::list<acc_scale_tests_t*>   compl_list;
};


int acc_scale_tests_push(void);

bool acc_scale_tests_blk_size_validate(const char *flag_name,
                                       uint64_t value);

}  // namespace tests

#endif /// ACC_SCALE_TESTS_HPP_
