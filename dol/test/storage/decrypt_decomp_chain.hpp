#ifndef	DECRYPT_DECOMP_CHAIN_HPP_
#define	DECRYPT_DECOMP_CHAIN_HPP_

#include "tests.hpp"
#include "xts.hpp"
#include "compression.hpp"

namespace tests {

class comp_encrypt_chain_t;


/*
 * Emulate named parameters support for decrypt_decomp_chain_t constructor
 */
class decrypt_decomp_chain_params_t
{
public:

    decrypt_decomp_chain_params_t() :
        app_max_size_(0),
        app_enc_size_(kCompAppHashBlkSize),
        uncomp_mem_type_(DP_MEM_TYPE_VOID),
        xts_status_mem_type1_(DP_MEM_TYPE_VOID),
        xts_status_mem_type2_(DP_MEM_TYPE_VOID),
        decrypt_mem_type_(DP_MEM_TYPE_VOID),
        destructor_free_buffers_(false),
        suppress_info_log_(false)
    {
    }

    uint32_t        app_max_size_;
    uint32_t        app_enc_size_;
    dp_mem_type_t   uncomp_mem_type_;
    dp_mem_type_t   xts_status_mem_type1_;
    dp_mem_type_t   xts_status_mem_type2_;
    dp_mem_type_t   decrypt_mem_type_;
    bool            destructor_free_buffers_;
    bool            suppress_info_log_;

    decrypt_decomp_chain_params_t&
    app_max_size(uint32_t app_max_size)
    {
        app_max_size_ = app_max_size;
        return *this;
    }
    decrypt_decomp_chain_params_t&
    app_enc_size(uint32_t app_enc_size)
    {
        app_enc_size_ = app_enc_size;
        return *this;
    }
    decrypt_decomp_chain_params_t&
    uncomp_mem_type(dp_mem_type_t uncomp_mem_type)
    {
        uncomp_mem_type_ = uncomp_mem_type;
        return *this;
    }
    decrypt_decomp_chain_params_t&
    xts_status_mem_type1(dp_mem_type_t xts_status_mem_type1)
    {
        xts_status_mem_type1_ = xts_status_mem_type1;
        return *this;
    }
    decrypt_decomp_chain_params_t&
    xts_status_mem_type2(dp_mem_type_t xts_status_mem_type2)
    {
        xts_status_mem_type2_ = xts_status_mem_type2;
        return *this;
    }
    decrypt_decomp_chain_params_t&
    decrypt_mem_type(dp_mem_type_t decrypt_mem_type)
    {
        decrypt_mem_type_ = decrypt_mem_type;
        return *this;
    }
    decrypt_decomp_chain_params_t&
    destructor_free_buffers(bool destructor_free_buffers)
    {
        destructor_free_buffers_ = destructor_free_buffers;
        return *this;
    }
    decrypt_decomp_chain_params_t&
    suppress_info_log(bool suppress_info_log)
    {
        suppress_info_log_ = suppress_info_log;
        return *this;
    }
};


/*
 * Emulate named parameters support for decrypt_decomp_chain_t pre_push
 */
class decrypt_decomp_chain_pre_push_params_t
{
public:

    decrypt_decomp_chain_pre_push_params_t() :
        caller_comp_status_buf_(nullptr),
        caller_comp_opaque_buf_(nullptr),
        caller_comp_opaque_data_(0)
    {
    }

    dp_mem_t    *caller_comp_status_buf_;
    dp_mem_t    *caller_comp_opaque_buf_;
    uint32_t    caller_comp_opaque_data_;

    decrypt_decomp_chain_pre_push_params_t&
    caller_comp_status_buf(dp_mem_t *caller_comp_status_buf)
    {
        caller_comp_status_buf_ = caller_comp_status_buf;
        return *this;
    }
    decrypt_decomp_chain_pre_push_params_t&
    caller_comp_opaque_buf(dp_mem_t *caller_comp_opaque_buf)
    {
        caller_comp_opaque_buf_ = caller_comp_opaque_buf;
        return *this;
    }
    decrypt_decomp_chain_pre_push_params_t&
    caller_comp_opaque_data(uint32_t caller_comp_opaque_data)
    {
        caller_comp_opaque_data_ = caller_comp_opaque_data;
        return *this;
    }
};


/*
 * Emulate named parameters support for decrypt_decomp_chain_t push
 */
class decrypt_decomp_chain_push_params_t
{
public:

    decrypt_decomp_chain_push_params_t() :
        comp_encrypt_chain_(nullptr),
        decomp_queue_(nullptr),
        seq_xts_qid_(0),
        seq_xts_status_qid_(0),
        seq_comp_status_qid_(0)
    {
    }

    comp_encrypt_chain_t    *comp_encrypt_chain_;
    comp_queue_t            *decomp_queue_;
    uint32_t                seq_xts_qid_;
    uint32_t                seq_xts_status_qid_;
    uint32_t                seq_comp_status_qid_;

    decrypt_decomp_chain_push_params_t&
    comp_encrypt_chain(comp_encrypt_chain_t *comp_encrypt_chain)
    {
        comp_encrypt_chain_ = comp_encrypt_chain;
        return *this;
    }
    decrypt_decomp_chain_push_params_t&
    decomp_queue(comp_queue_t *decomp_queue)
    {
        decomp_queue_ = decomp_queue;
        return *this;
    }
    decrypt_decomp_chain_push_params_t&
    seq_xts_qid(uint32_t seq_xts_qid)
    {
        seq_xts_qid_ = seq_xts_qid;
        return *this;
    }
    decrypt_decomp_chain_push_params_t&
    seq_xts_status_qid(uint32_t seq_xts_status_qid)
    {
        seq_xts_status_qid_ = seq_xts_status_qid;
        return *this;
    }
    decrypt_decomp_chain_push_params_t&
    seq_comp_status_qid(uint32_t seq_comp_status_qid)
    {
        seq_comp_status_qid_ = seq_comp_status_qid;
        return *this;
    }
};


/*
 * Accelerator XTS-decrypt to decompression chaining DOL
 */
class decrypt_decomp_chain_t
{
public:

    decrypt_decomp_chain_t(decrypt_decomp_chain_params_t params);
    ~decrypt_decomp_chain_t();

    void pre_push(decrypt_decomp_chain_pre_push_params_t params);
    int  push(decrypt_decomp_chain_push_params_t params);
    void post_push(void);
    int  fast_verify(void);
    int  full_verify(void);

    uint32_t dc_output_data_len_get(void)
    {
        return last_dc_output_data_len;
    }

    uint32_t decrypt_output_data_len_get(void)
    {
        return last_decrypt_output_data_len;
    }

private:

    void  decrypt_setup(uint32_t block_no,
                        acc_chain_params_t& chain_params);

    comp_encrypt_chain_t    *comp_encrypt_chain;
    xts_enc_dec_blk_type_t  enc_dec_blk_type;
    uint32_t                app_max_size;
    uint32_t                app_blk_size;
    uint32_t                app_enc_size;

    // Buffers used for decryption->decompression operations
    dp_mem_t                *uncomp_buf;
    dp_mem_t                *xts_decrypt_buf;

    dp_mem_t                *xts_status_vec1;
    dp_mem_t                *xts_status_vec2;
    dp_mem_t                *xts_opaque_buf;

    // XTS uses AOL for input/output;
    dp_mem_t                *xts_src_aol_vec;
    dp_mem_t                *xts_dst_aol_vec;
    dp_mem_t                *xts_desc_vec;

    // Decomp descriptor for use by XTS-decrypt to decomp chaining
    dp_mem_t                *xts_decomp_cp_desc;

    // Since Comp is last in the chain, allow the caller to supply
    // their own opaque and status buffers
    dp_mem_t                *caller_comp_status_buf;
    dp_mem_t                *caller_comp_opaque_buf;
    uint32_t                caller_comp_opaque_data;

    // other context info
    cp_desc_t               cp_desc;
    XtsCtx                  xts_ctx;
    comp_queue_t            *decomp_queue;

    int                     actual_enc_blks;
    uint32_t                max_enc_blks;
    uint32_t                num_enc_blks;

    uint32_t                last_dc_output_data_len;
    uint32_t                last_decrypt_output_data_len;

    bool                    destructor_free_buffers;
    bool                    suppress_info_log;
    bool                    success;
};


}  // namespace tests

#endif  // DECRYPT_DECOMP_CHAIN_HPP_
