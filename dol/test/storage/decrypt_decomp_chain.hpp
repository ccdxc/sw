#ifndef	DECRYPT_DECOMP_CHAIN_HPP_
#define	DECRYPT_DECOMP_CHAIN_HPP_

#include "tests.hpp"
#include "xts.hpp"
#include "compression.hpp"

namespace tests {

class comp_encrypt_chain_t;


/*
 * Compression length is either a known quantity, allowing the decomp descriptor
 * to be fully set up prior to handing it to P4+, or unknown. For the latter,
 * P4+ can read the cp_hdr from the decrypted data and fill in the length
 * for the decomp descriptor. It can do so for any of the following 3 situations:
 * 1) decomp buffer is a flat buffer, or
 * 2) decomp buffer is described with a single source SGL, or
 * 3) decomp buffer is described with a sparse vector of source SGLs.
 */
typedef enum {
    DECRYPT_DECOMP_LEN_UPDATE_NONE,
    DECRYPT_DECOMP_LEN_UPDATE_FLAT,
    DECRYPT_DECOMP_LEN_UPDATE_SGL_SRC,
    DECRYPT_DECOMP_LEN_UPDATE_SGL_SRC_VEC
} decrypt_decomp_len_update_t;


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
        decrypt_mem_type1_(DP_MEM_TYPE_VOID),
        decrypt_mem_type2_(DP_MEM_TYPE_VOID),
        destructor_free_buffers_(false),
        suppress_info_log_(false)
    {
    }

    uint32_t        app_max_size_;
    uint32_t        app_enc_size_;
    dp_mem_type_t   uncomp_mem_type_;
    dp_mem_type_t   xts_status_mem_type1_;
    dp_mem_type_t   xts_status_mem_type2_;
    dp_mem_type_t   decrypt_mem_type1_;
    dp_mem_type_t   decrypt_mem_type2_;
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
    decrypt_mem_type1(dp_mem_type_t decrypt_mem_type1)
    {
        decrypt_mem_type1_ = decrypt_mem_type1;
        return *this;
    }
    decrypt_decomp_chain_params_t&
    decrypt_mem_type2(dp_mem_type_t decrypt_mem_type2)
    {
        decrypt_mem_type2_ = decrypt_mem_type2;
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
        decomp_ring_(nullptr),
        seq_xts_qid_(0),
        seq_xts_status_qid_(0),
        seq_comp_status_qid_(0),
        decrypt_decomp_len_update_(DECRYPT_DECOMP_LEN_UPDATE_NONE)
    {
    }

    comp_encrypt_chain_t        *comp_encrypt_chain_;
    acc_ring_t                  *decomp_ring_;
    uint32_t                    seq_xts_qid_;
    uint32_t                    seq_xts_status_qid_;
    uint32_t                    seq_comp_status_qid_;
    decrypt_decomp_len_update_t decrypt_decomp_len_update_;

    decrypt_decomp_chain_push_params_t&
    comp_encrypt_chain(comp_encrypt_chain_t *comp_encrypt_chain)
    {
        comp_encrypt_chain_ = comp_encrypt_chain;
        return *this;
    }
    decrypt_decomp_chain_push_params_t&
    decomp_ring(acc_ring_t *decomp_ring)
    {
        decomp_ring_ = decomp_ring;
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
    decrypt_decomp_chain_push_params_t&
    decrypt_decomp_len_update(decrypt_decomp_len_update_t decrypt_decomp_len_update)
    {
        decrypt_decomp_len_update_ = decrypt_decomp_len_update;
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
                        chain_params_xts_t& chain_params);

    comp_encrypt_chain_t    *comp_encrypt_chain;
    xts_enc_dec_blk_type_t  enc_dec_blk_type;
    uint32_t                app_max_size;
    uint32_t                app_blk_size;
    uint32_t                app_enc_size;

    // Buffers used for decryption->decompression operations
    dp_mem_t                *uncomp_buf;
    dp_mem_t                *xts_decrypt_buf1;
    dp_mem_t                *xts_decrypt_buf2;
    dp_mem_t                *seq_sgl_pdma;

    dp_mem_t                *xts_status_vec1;
    dp_mem_t                *xts_status_vec2;
    dp_mem_t                *xts_opaque_buf;

    // XTS uses AOL for input/output;
    dp_mem_t                *xts_src_aol_vec;
    dp_mem_t                *xts_dst_aol_vec;
    dp_mem_t                *xts_desc_vec;

    // Decomp descriptor for use by XTS-decrypt to decomp chaining
    dp_mem_t                *xts_decomp_cp_desc;
    dp_mem_t                *decomp_sgl_src_vec;

    // Since Comp is last in the chain, allow the caller to supply
    // their own opaque and status buffers
    dp_mem_t                *caller_comp_status_buf;
    dp_mem_t                *caller_comp_opaque_buf;
    uint32_t                caller_comp_opaque_data;

    // other context info
    cp_desc_t               cp_desc;
    XtsCtx                  xts_ctx;
    acc_ring_t              *decomp_ring;

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
