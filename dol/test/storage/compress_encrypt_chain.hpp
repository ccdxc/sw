#ifndef	COMP_ENCRYPT_CHAIN_HPP_
#define	COMP_ENCRYPT_CHAIN_HPP_

#include "tests.hpp"
#include "xts.hpp"
#include "compression.hpp"

/*
 * Accelerator compression to XTS-encrypt chaining DOL
 */
namespace tests {

/*
 * Emulate named parameters support for constructor
 */
struct comp_encrypt_chain_params_t {
    uint32_t        app_max_size_;
    dp_mem_type_t   uncomp_mem_type_;
    dp_mem_type_t   comp_mem_type_;
    dp_mem_type_t   comp_status_mem_type1_;
    dp_mem_type_t   comp_status_mem_type2_;
    dp_mem_type_t   encrypt_mem_type_;
    bool            destructor_free_buffers_;

    comp_encrypt_chain_params_t&
    app_max_size(uint32_t app_max_size)
    {
        app_max_size_ = app_max_size;
        return *this;
    }
    comp_encrypt_chain_params_t&
    uncomp_mem_type(dp_mem_type_t uncomp_mem_type)
    {
        uncomp_mem_type_ = uncomp_mem_type;
        return *this;
    }
    comp_encrypt_chain_params_t&
    comp_mem_type(dp_mem_type_t comp_mem_type)
    {
        comp_mem_type_ = comp_mem_type;
        return *this;
    }
    comp_encrypt_chain_params_t&
    comp_status_mem_type1(dp_mem_type_t comp_status_mem_type1)
    {
        comp_status_mem_type1_ = comp_status_mem_type1;
        return *this;
    }
    comp_encrypt_chain_params_t
    &comp_status_mem_type2(dp_mem_type_t comp_status_mem_type2)
    {
        comp_status_mem_type2_ = comp_status_mem_type2;
        return *this;
    }
    comp_encrypt_chain_params_t&
    encrypt_mem_type(dp_mem_type_t encrypt_mem_type)
    {
        encrypt_mem_type_ = encrypt_mem_type;
        return *this;
    }
    comp_encrypt_chain_params_t&
    destructor_free_buffers(bool destructor_free_buffers)
    {
        destructor_free_buffers_ = destructor_free_buffers;
        return *this;
    }
};


/*
 * Emulate named parameters support for pre_push
 */
struct comp_encrypt_chain_pre_push_params_t {
    dp_mem_t    *caller_xts_status_buf_;
    dp_mem_t    *caller_xts_opaque_buf_;
    uint64_t    caller_xts_opaque_data_;

    comp_encrypt_chain_pre_push_params_t&
    caller_xts_status_buf(dp_mem_t *caller_xts_status_buf)
    {
        caller_xts_status_buf_ = caller_xts_status_buf;
        return *this;
    }
    comp_encrypt_chain_pre_push_params_t&
    caller_xts_opaque_buf(dp_mem_t *caller_xts_opaque_buf)
    {
        caller_xts_opaque_buf_ = caller_xts_opaque_buf;
        return *this;
    }
    comp_encrypt_chain_pre_push_params_t&
    caller_xts_opaque_data(uint64_t caller_xts_opaque_data)
    {
        caller_xts_opaque_data_ = caller_xts_opaque_data;
        return *this;
    }
};


class comp_encrypt_chain_t
{
public:

    comp_encrypt_chain_t(uint32_t app_max_size,
                         dp_mem_type_t uncomp_mem_type,
                         dp_mem_type_t comp_mem_type,
                         dp_mem_type_t comp_status_mem_type1,
                         dp_mem_type_t comp_status_mem_type2,
                         dp_mem_type_t encrypt_mem_type);
    comp_encrypt_chain_t(comp_encrypt_chain_params_t params);
    ~comp_encrypt_chain_t();

    void pre_push(comp_encrypt_chain_pre_push_params_t params);
    int  push(uint32_t app_blk_size,
              comp_queue_t *cp_queue,
              comp_queue_push_t push_type,
              uint32_t seq_comp_qid,
              uint32_t seq_comp_status_qid,
              uint32_t seq_xts_status_qid);
    void post_push(comp_queue_t *cp_queue);
    int  verify(void);

    uint32_t cp_output_data_len_get(void)
    {
        return last_cp_output_data_len;
    }

    uint32_t encrypt_output_data_len_get(void)
    {
        return last_encrypt_output_data_len;
    }

private:

    void  encrypt_setup(acc_chain_params_t& chain_params);

    uint32_t    app_max_size;

    // Buffers used for compression->encryption operations
    dp_mem_t    *uncomp_buf;
    dp_mem_t    *comp_buf;
    dp_mem_t    *xts_encrypt_buf;

    dp_mem_t    *comp_status_buf1;
    dp_mem_t    *comp_status_buf2;
    dp_mem_t    *comp_opaque_buf;

    // XTS uses AOL for input/output;
    dp_mem_t    *xts_in_aol;
    dp_mem_t    *xts_out_aol;
    dp_mem_t    *xts_desc_buf;

    // Since XTS is last in the chain, allow the caller to supply
    // their own opaque and status buffers
    dp_mem_t    *caller_xts_status_buf;
    dp_mem_t    *caller_xts_opaque_buf;
    uint64_t    caller_xts_opaque_data;

    // other context info
    cp_desc_t   cp_desc;
    XtsCtx      xts_ctx;

    uint32_t    last_cp_output_data_len;
    uint32_t    last_encrypt_output_data_len;

    bool        success;
    bool        destructor_free_buffers;
};


}  // namespace tests

#endif  // COMP_ENCRYPT_CHAIN_HPP_
