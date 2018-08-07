#ifndef	ENCRYPT_ONLY_HPP_
#define	ENCRYPT_ONLY_HPP_

#include "tests.hpp"
#include "xts.hpp"

namespace tests {

/*
 * Emulate named parameters support for encrypt_only_t constructor
 */
class encrypt_only_params_t
{
public:

    encrypt_only_params_t() :
        app_max_size_(0),
        unencrypt_mem_type_(DP_MEM_TYPE_VOID),
        encrypt_mem_type_(DP_MEM_TYPE_VOID),
        destructor_free_buffers_(false),
        suppress_info_log_(false)
    {
    }

    uint32_t        app_max_size_;
    dp_mem_type_t   unencrypt_mem_type_;
    dp_mem_type_t   encrypt_mem_type_;
    bool            destructor_free_buffers_;
    bool            suppress_info_log_;

    encrypt_only_params_t&
    app_max_size(uint32_t app_max_size)
    {
        app_max_size_ = app_max_size;
        return *this;
    }
    encrypt_only_params_t&
    unencrypt_mem_type(dp_mem_type_t unencrypt_mem_type)
    {
        unencrypt_mem_type_ = unencrypt_mem_type;
        return *this;
    }
    encrypt_only_params_t&
    encrypt_mem_type(dp_mem_type_t encrypt_mem_type)
    {
        encrypt_mem_type_ = encrypt_mem_type;
        return *this;
    }
    encrypt_only_params_t&
    destructor_free_buffers(bool destructor_free_buffers)
    {
        destructor_free_buffers_ = destructor_free_buffers;
        return *this;
    }
    encrypt_only_params_t&
    suppress_info_log(bool suppress_info_log)
    {
        suppress_info_log_ = suppress_info_log;
        return *this;
    }
};


/*
 * Emulate named parameters support for encrypt_only_t pre_push
 */
class encrypt_only_pre_push_params_t
{
public:

    encrypt_only_pre_push_params_t() :
        caller_xts_status_buf_(nullptr),
        caller_xts_opaque_buf_(nullptr),
        caller_xts_opaque_data_(0)
    {
    }

    dp_mem_t    *caller_xts_status_buf_;
    dp_mem_t    *caller_xts_opaque_buf_;
    uint64_t    caller_xts_opaque_data_;

    encrypt_only_pre_push_params_t&
    caller_xts_status_buf(dp_mem_t *caller_xts_status_buf)
    {
        caller_xts_status_buf_ = caller_xts_status_buf;
        return *this;
    }
    encrypt_only_pre_push_params_t&
    caller_xts_opaque_buf(dp_mem_t *caller_xts_opaque_buf)
    {
        caller_xts_opaque_buf_ = caller_xts_opaque_buf;
        return *this;
    }
    encrypt_only_pre_push_params_t&
    caller_xts_opaque_data(uint64_t caller_xts_opaque_data)
    {
        caller_xts_opaque_data_ = caller_xts_opaque_data;
        return *this;
    }
};


/*
 * Emulate named parameters support for encrypt_only_t push
 */
class encrypt_only_push_params_t
{
public:

    encrypt_only_push_params_t() :
        app_blk_size_(0),
        push_type_(ACC_RING_PUSH_SEQUENCER),
        seq_xts_qid_(0)
    {
    }

    uint32_t            app_blk_size_;
    acc_ring_push_t     push_type_;
    uint32_t            seq_xts_qid_;

    encrypt_only_push_params_t&
    app_blk_size(uint32_t app_blk_size)
    {
        app_blk_size_ = app_blk_size;
        return *this;
    }
    encrypt_only_push_params_t&
    push_type(acc_ring_push_t push_type)
    {
        push_type_ = push_type;
        return *this;
    }
    encrypt_only_push_params_t&
    seq_xts_qid(uint32_t seq_xts_qid)
    {
        seq_xts_qid_ = seq_xts_qid;
        return *this;
    }
};


/*
 * Accelerator compression to XTS-encrypt chaining DOL
 */
class encrypt_only_t
{
public:

    encrypt_only_t(encrypt_only_params_t params);
    ~encrypt_only_t();

    void pre_push(encrypt_only_pre_push_params_t params);
    int  push(encrypt_only_push_params_t params);
    void post_push(void);
    int  fast_verify(void);
    int  full_verify(void);

    uint32_t app_blk_size_get(void)
    {
        return app_blk_size;
    }

    uint32_t encrypt_output_data_len_get(void)
    {
        return last_encrypt_output_data_len;
    }

    dp_mem_t *xts_encrypt_buf_get(void)
    {
        return xts_encrypt_buf;
    }


private:

    void  encrypt_setup(uint32_t seq_xts_qid);

    uint32_t        app_max_size;
    uint32_t        app_blk_size;

    // Buffers used for compression->encryption operations
    dp_mem_t        *unencrypt_buf;
    dp_mem_t        *xts_encrypt_buf;

    dp_mem_t        *xts_status_buf;

    // XTS uses AOL for input/output;
    dp_mem_t        *xts_in_aol;
    dp_mem_t        *xts_out_aol;
    dp_mem_t        *xts_desc_buf;

    // Since XTS is last in the chain, allow the caller to supply
    // their own opaque and status buffers
    dp_mem_t        *caller_xts_status_buf;
    dp_mem_t        *caller_xts_opaque_buf;
    uint64_t        caller_xts_opaque_data;

    // other context info
    XtsCtx          xts_ctx;

    uint32_t        last_encrypt_output_data_len;

    bool            destructor_free_buffers;
    bool            suppress_info_log;
    bool            success;
};


}  // namespace tests

#endif  // ENCRYPT_ONLY_HPP_
