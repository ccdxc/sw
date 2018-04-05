#ifndef	ACC_SCALE_TESTS_HPP_
#define	ACC_SCALE_TESTS_HPP_

#include <vector>
#include <list>
#include "decrypt_decomp_chain.hpp"
#include "comp_encrypt_chain.hpp"

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
    }

    virtual ~acc_scale_tests_t()
    {
    }

    virtual ~acc_scale_tests_t()
    {
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

protected:
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

    const char      *scale_test_name;
    decrypt_decomp_chain_push_params_t  push_params;

    dp_mem_t        *comp_status_host_buf;
    dp_mem_t        *comp_opaque_host_buf;

    dp_mem_t        *exp_opaque_data_buf;
    uint32_t        exp_opaque_data;

    bool            destructor_free_buffers;
    bool            success;
};


/*
 * List of accelerator scale tests
 */
class acc_scale_tests_list_t
{
public:

    acc_scale_tests_list_t();
    ~acc_scale_tests_list_t();

    void add(acc_scale_tests_t *scale_test);
    int run(const char *test_name);
    int completion_check(void);
    void outstanding_report(void);

private:
    std::list<acc_scale_tests_t*>   tests_list;

    uint32_t        num_tests_completed;
    uint32_t        num_tests_failed;
};


int acc_scale_tests_comp_encrypt_decrypt_decomp(void);

bool acc_scale_tests_blk_size_validate(const char *flag_name,
                                       uint64_t value);

}  // namespace tests

#endif /// ACC_SCALE_TESTS_HPP_
