#ifndef	COMP_ENCRYPT_CHAIN_HPP_
#define	COMP_ENCRYPT_CHAIN_HPP_

#include "tests.hpp"
#include "xts.hpp"
#include "compression.hpp"

namespace tests {

/*
 * Emulate named parameters support for comp_encrypt_chain_t constructor
 */
class comp_encrypt_chain_params_t
{
public:

    comp_encrypt_chain_params_t() :
        app_max_size_(0),
        app_enc_size_(kCompAppHashBlkSize),
        uncomp_mem_type_(DP_MEM_TYPE_VOID),
        comp_mem_type_(DP_MEM_TYPE_VOID),
        comp_status_mem_type1_(DP_MEM_TYPE_VOID),
        comp_status_mem_type2_(DP_MEM_TYPE_VOID),
        encrypt_mem_type_(DP_MEM_TYPE_VOID),
        destructor_free_buffers_(false),
        suppress_info_log_(false)
    {
    }

    uint32_t        app_max_size_;
    uint32_t        app_enc_size_;
    dp_mem_type_t   uncomp_mem_type_;
    dp_mem_type_t   comp_mem_type_;
    dp_mem_type_t   comp_status_mem_type1_;
    dp_mem_type_t   comp_status_mem_type2_;
    dp_mem_type_t   encrypt_mem_type_;
    bool            destructor_free_buffers_;
    bool            suppress_info_log_;

    comp_encrypt_chain_params_t&
    app_max_size(uint32_t app_max_size)
    {
        app_max_size_ = app_max_size;
        return *this;
    }
    comp_encrypt_chain_params_t&
    app_enc_size(uint32_t app_enc_size)
    {
        app_enc_size_ = app_enc_size;
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
    comp_encrypt_chain_params_t&
    comp_status_mem_type2(dp_mem_type_t comp_status_mem_type2)
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
    comp_encrypt_chain_params_t&
    suppress_info_log(bool suppress_info_log)
    {
        suppress_info_log_ = suppress_info_log;
        return *this;
    }
};


/*
 * Emulate named parameters support for comp_encrypt_chain_t pre_push
 */
class comp_encrypt_chain_pre_push_params_t
{
public:

    comp_encrypt_chain_pre_push_params_t() :
        caller_comp_pad_buf_(nullptr),
        caller_xts_status_vec_(nullptr),
        caller_xts_opaque_vec_(nullptr),
        caller_xts_opaque_data_(0)
    {
    }

    dp_mem_t    *caller_comp_pad_buf_;
    dp_mem_t    *caller_xts_status_vec_;
    dp_mem_t    *caller_xts_opaque_vec_;
    uint64_t    caller_xts_opaque_data_;

    comp_encrypt_chain_pre_push_params_t&
    caller_comp_pad_buf(dp_mem_t *caller_comp_pad_buf)
    {
        caller_comp_pad_buf_ = caller_comp_pad_buf;
        return *this;
    }
    comp_encrypt_chain_pre_push_params_t&
    caller_xts_status_vec(dp_mem_t *caller_xts_status_vec)
    {
        caller_xts_status_vec_ = caller_xts_status_vec;
        return *this;
    }
    comp_encrypt_chain_pre_push_params_t&
    caller_xts_opaque_vec(dp_mem_t *caller_xts_opaque_vec)
    {
        caller_xts_opaque_vec_ = caller_xts_opaque_vec;
        return *this;
    }
    comp_encrypt_chain_pre_push_params_t&
    caller_xts_opaque_data(uint64_t caller_xts_opaque_data)
    {
        caller_xts_opaque_data_ = caller_xts_opaque_data;
        return *this;
    }
};


/*
 * Emulate named parameters support for comp_encrypt_chain_t push
 */
class comp_encrypt_chain_push_params_t
{
public:

    comp_encrypt_chain_push_params_t() :
        enc_dec_blk_type_(XTS_ENC_DEC_ENTIRE_APP_BLK),
        app_blk_size_(0),
        comp_ring_(nullptr),
        push_type_(ACC_RING_PUSH_SEQUENCER),
        seq_comp_qid_(0),
        seq_comp_status_qid_(0),
        seq_xts_status_qid_(0)
    {
    }

    xts_enc_dec_blk_type_t enc_dec_blk_type_;
    uint32_t            app_blk_size_;
    acc_ring_t          *comp_ring_;
    acc_ring_push_t     push_type_;
    uint32_t            seq_comp_qid_;
    uint32_t            seq_comp_status_qid_;
    uint32_t            seq_xts_status_qid_;

    comp_encrypt_chain_push_params_t&
    enc_dec_blk_type(xts_enc_dec_blk_type_t enc_dec_blk_type)
    {
        enc_dec_blk_type_ = enc_dec_blk_type;
        return *this;
    }
    comp_encrypt_chain_push_params_t&
    app_blk_size(uint32_t app_blk_size)
    {
        app_blk_size_ = app_blk_size;
        return *this;
    }
    comp_encrypt_chain_push_params_t&
    comp_ring(acc_ring_t *comp_ring)
    {
        comp_ring_ = comp_ring;
        return *this;
    }
    comp_encrypt_chain_push_params_t&
    push_type(acc_ring_push_t push_type)
    {
        push_type_ = push_type;
        return *this;
    }
    comp_encrypt_chain_push_params_t&
    seq_comp_qid(uint32_t seq_comp_qid)
    {
        seq_comp_qid_ = seq_comp_qid;
        return *this;
    }
    comp_encrypt_chain_push_params_t&
    seq_comp_status_qid(uint32_t seq_comp_status_qid)
    {
        seq_comp_status_qid_ = seq_comp_status_qid;
        return *this;
    }
    comp_encrypt_chain_push_params_t&
    seq_xts_status_qid(uint32_t seq_xts_status_qid)
    {
        seq_xts_status_qid_ = seq_xts_status_qid;
        return *this;
    }
};


/*
 * Accelerator compression to XTS-encrypt chaining DOL
 */
class comp_encrypt_chain_t
{
public:

    comp_encrypt_chain_t(comp_encrypt_chain_params_t params);
    ~comp_encrypt_chain_t();

    void pre_push(comp_encrypt_chain_pre_push_params_t params);
    int  push(comp_encrypt_chain_push_params_t params);
    void post_push(void);
    int  fast_verify(void);
    int  full_verify(void);

    uint32_t app_blk_size_get(void)
    {
        return app_blk_size;
    }

    uint32_t app_enc_size_get(void)
    {
        return app_enc_size;
    }

    xts_enc_dec_blk_type_t enc_dec_blk_type_get(void)
    {
        return enc_dec_blk_type;
    }

    uint32_t cp_output_data_len_get(void)
    {
        return last_cp_output_data_len;
    }

    uint32_t encrypt_output_data_len_get(void)
    {
        return last_encrypt_output_data_len;
    }

    dp_mem_t *xts_encrypt_buf_get(void)
    {
        return xts_encrypt_buf;
    }

    uint8_t *uncomp_data_get(void)
    {
        return uncomp_buf->read_thru();
    }

    uint8_t *comp_data_get(void)
    {
        return comp_buf->read_thru();
    }

    int actual_enc_blks_get(test_resource_query_method_t query_method);

private:

    void  encrypt_setup(uint32_t block_no,
                        chain_params_comp_t& chain_params);

    xts_enc_dec_blk_type_t enc_dec_blk_type;
    uint32_t        app_max_size;
    uint32_t        app_blk_size;
    uint32_t        app_enc_size;

    // Buffers used for compression->encryption operations
    dp_mem_t        *uncomp_buf;
    dp_mem_t        *comp_buf;
    dp_mem_t        *xts_encrypt_buf;

    dp_mem_t        *comp_status_buf1;
    dp_mem_t        *comp_status_buf2;
    dp_mem_t        *comp_opaque_buf;

    // XTS uses AOL for input/output;
    dp_mem_t        *xts_src_aol_vec;
    dp_mem_t        *xts_dst_aol_vec;
    dp_mem_t        *xts_desc_vec;
    dp_mem_t        *xts_opaque_vec;

    // Since XTS is last in the chain, allow the caller to supply
    // their own opaque and status buffers
    dp_mem_t        *caller_comp_pad_buf;
    dp_mem_t        *caller_xts_status_vec;
    dp_mem_t        *caller_xts_opaque_vec;
    uint64_t        caller_xts_opaque_data;

    // other context info
    cp_desc_t       cp_desc;
    XtsCtx          xts_ctx;
    acc_ring_t      *comp_ring;
    acc_ring_push_t push_type;
    uint32_t        seq_comp_qid;

    int             actual_enc_blks;
    uint32_t        max_enc_blks;
    uint32_t        num_enc_blks;

    uint32_t        last_cp_output_data_len;
    uint32_t        last_encrypt_output_data_len;

    bool            destructor_free_buffers;
    bool            suppress_info_log;
    bool            success;
};


}  // namespace tests

#endif  // COMP_ENCRYPT_CHAIN_HPP_
