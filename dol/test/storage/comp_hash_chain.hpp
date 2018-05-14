#ifndef	COMP_HASH_CHAIN_HPP_
#define	COMP_HASH_CHAIN_HPP_

#include "tests.hpp"
#include "compression.hpp"

#include <vector>

namespace tests {

/*
 * Emulate named parameters support for comp_hash_chain_t constructor
 */
class comp_hash_chain_params_t
{
public:

    comp_hash_chain_params_t() :
        app_max_size_(0),
        uncomp_mem_type_(DP_MEM_TYPE_VOID),
        comp_mem_type1_(DP_MEM_TYPE_VOID),
        comp_mem_type2_(DP_MEM_TYPE_VOID),
        comp_status_mem_type1_(DP_MEM_TYPE_VOID),
        comp_status_mem_type2_(DP_MEM_TYPE_VOID),
        destructor_free_buffers_(false),
        suppress_info_log_(false)
    {
    }

    uint32_t        app_max_size_;
    dp_mem_type_t   uncomp_mem_type_;
    dp_mem_type_t   comp_mem_type1_;
    dp_mem_type_t   comp_mem_type2_;
    dp_mem_type_t   comp_status_mem_type1_;
    dp_mem_type_t   comp_status_mem_type2_;
    bool            destructor_free_buffers_;
    bool            suppress_info_log_;

    comp_hash_chain_params_t&
    app_max_size(uint32_t app_max_size)
    {
        app_max_size_ = app_max_size;
        return *this;
    }
    comp_hash_chain_params_t&
    uncomp_mem_type(dp_mem_type_t uncomp_mem_type)
    {
        uncomp_mem_type_ = uncomp_mem_type;
        return *this;
    }
    comp_hash_chain_params_t&
    comp_mem_type1(dp_mem_type_t comp_mem_type1)
    {
        comp_mem_type1_ = comp_mem_type1;
        return *this;
    }
    comp_hash_chain_params_t&
    comp_mem_type2(dp_mem_type_t comp_mem_type2)
    {
        comp_mem_type2_ = comp_mem_type2;
        return *this;
    }
    comp_hash_chain_params_t&
    comp_status_mem_type1(dp_mem_type_t comp_status_mem_type1)
    {
        comp_status_mem_type1_ = comp_status_mem_type1;
        return *this;
    }
    comp_hash_chain_params_t&
    comp_status_mem_type2(dp_mem_type_t comp_status_mem_type2)
    {
        comp_status_mem_type2_ = comp_status_mem_type2;
        return *this;
    }
    comp_hash_chain_params_t&
    destructor_free_buffers(bool destructor_free_buffers)
    {
        destructor_free_buffers_ = destructor_free_buffers;
        return *this;
    }
    comp_hash_chain_params_t&
    suppress_info_log(bool suppress_info_log)
    {
        suppress_info_log_ = suppress_info_log;
        return *this;
    }
};


/*
 * Emulate named parameters support for comp_hash_chain_t pre_push
 */
class comp_hash_chain_pre_push_params_t
{
public:

    comp_hash_chain_pre_push_params_t() :
        caller_comp_pad_buf_(nullptr),
        caller_hash_status_vec_(nullptr),
        caller_hash_opaque_vec_(nullptr),
        caller_hash_opaque_data_(0)
    {
    }

    dp_mem_t    *caller_comp_pad_buf_;
    dp_mem_t    *caller_hash_status_vec_;
    dp_mem_t    *caller_hash_opaque_vec_;
    uint32_t    caller_hash_opaque_data_;

    comp_hash_chain_pre_push_params_t&
    caller_comp_pad_buf(dp_mem_t *caller_comp_pad_buf)
    {
        caller_comp_pad_buf_ = caller_comp_pad_buf;
        return *this;
    }
    comp_hash_chain_pre_push_params_t&
    caller_hash_status_vec(dp_mem_t *caller_hash_status_vec)
    {
        caller_hash_status_vec_ = caller_hash_status_vec;
        return *this;
    }
    comp_hash_chain_pre_push_params_t&
    caller_hash_opaque_vec(dp_mem_t *caller_hash_opaque_vec)
    {
        caller_hash_opaque_vec_ = caller_hash_opaque_vec;
        return *this;
    }
    comp_hash_chain_pre_push_params_t&
    caller_hash_opaque_data(uint32_t caller_hash_opaque_data)
    {
        caller_hash_opaque_data_ = caller_hash_opaque_data;
        return *this;
    }
};


/*
 * Emulate named parameters support for comp_hash_chain_t push
 */
class comp_hash_chain_push_params_t
{
public:

    comp_hash_chain_push_params_t() :
        app_blk_size_(0),
        app_hash_size_(kCompAppHashBlkSize),
        sha_en_(true),
        sha_type_(COMP_DEDUP_SHA512),
        integrity_src_(COMP_INTEGRITY_SRC_UNCOMP_DATA),
        integrity_type_(COMP_INTEGRITY_M_CRC64),
        comp_queue_(nullptr),
        hash_queue_(nullptr),
        push_type_(COMP_QUEUE_PUSH_SEQUENCER),
        seq_comp_qid_(0),
        seq_comp_status_qid_(0)
    {
    }

    uint32_t            app_blk_size_;
    uint32_t            app_hash_size_;
    uint8_t             sha_en_;
    uint8_t             sha_type_;
    uint8_t             integrity_src_;
    uint8_t             integrity_type_;
    comp_queue_t        *comp_queue_;
    comp_queue_t        *hash_queue_;
    comp_queue_push_t   push_type_;
    uint32_t            seq_comp_qid_;
    uint32_t            seq_comp_status_qid_;

    comp_hash_chain_push_params_t&
    app_blk_size(uint32_t app_blk_size)
    {
        app_blk_size_ = app_blk_size;
        return *this;
    }
    comp_hash_chain_push_params_t&
    app_hash_size(uint32_t app_hash_size)
    {
        app_hash_size_ = app_hash_size;
        return *this;
    }
    comp_hash_chain_push_params_t&
    sha_en(uint8_t sha_en)
    {
        sha_en_ = sha_en;
        return *this;
    }
    comp_hash_chain_push_params_t&
    sha_type(uint8_t sha_type)
    {
        sha_type_ = sha_type;
        return *this;
    }
    comp_hash_chain_push_params_t&
    integrity_src(uint8_t integrity_src)
    {
        integrity_src_ = integrity_src;
        return *this;
    }
    comp_hash_chain_push_params_t&
    integrity_type(uint8_t integrity_type)
    {
        integrity_type_ = integrity_type;
        return *this;
    }
    comp_hash_chain_push_params_t&
    comp_queue(comp_queue_t *comp_queue)
    {
        comp_queue_ = comp_queue;
        return *this;
    }
    comp_hash_chain_push_params_t&
    hash_queue(comp_queue_t *hash_queue)
    {
        hash_queue_ = hash_queue;
        return *this;
    }
    comp_hash_chain_push_params_t&
    push_type(comp_queue_push_t push_type)
    {
        push_type_ = push_type;
        return *this;
    }
    comp_hash_chain_push_params_t&
    seq_comp_qid(uint32_t seq_comp_qid)
    {
        seq_comp_qid_ = seq_comp_qid;
        return *this;
    }
    comp_hash_chain_push_params_t&
    seq_comp_status_qid(uint32_t seq_comp_status_qid)
    {
        seq_comp_status_qid_ = seq_comp_status_qid;
        return *this;
    }
};

/*
 * Accelerator compression to hash (for dedup) chaining DOL
 */
class comp_hash_chain_t
{
public:

    comp_hash_chain_t(comp_hash_chain_params_t params);
    ~comp_hash_chain_t();

    void pre_push(comp_hash_chain_pre_push_params_t params);
    int  push(comp_hash_chain_push_params_t params);
    void post_push(void);
    int  fast_verify(void);
    int  full_verify(void);

    uint32_t app_blk_size_get(void)
    {
        return app_blk_size;
    }

    uint32_t app_hash_size_get(void)
    {
        return app_hash_size;
    }

    uint32_t cp_output_data_len_get(void)
    {
        return last_cp_output_data_len;
    }

    dp_mem_t *uncomp_buf_get(void)
    {
        return uncomp_buf;
    }

    dp_mem_t *comp_buf1_get(void)
    {
        return comp_buf1;
    }

    dp_mem_t *comp_buf2_get(void)
    {
        return comp_buf2;
    }

    dp_mem_t *hash_status_vec_get(void)
    {
        return caller_hash_status_vec;
    }

    uint8_t sha_en_get(void)
    {
        return sha_en;
    }

    uint8_t sha_type_get(void)
    {
        return sha_type;
    }

    uint8_t integrity_src_get(void)
    {
        return integrity_src;
    }

    uint8_t integrity_type_get(void)
    {
        return integrity_type;
    }

    int actual_hash_blks_get(test_resource_query_method_t query_method);

private:

    void  hash_setup(uint32_t block_no,
                     acc_chain_params_t& chain_params);

    uint32_t        app_max_size;
    uint32_t        app_blk_size;
    uint32_t        app_hash_size;
    uint32_t        max_hash_blks;
    uint32_t        num_hash_blks;
    int             actual_hash_blks;   // -1 means unknown

    // Buffers used for compression operations
    dp_mem_t        *uncomp_buf;
    dp_mem_t        *comp_buf1;
    dp_mem_t        *comp_buf2;
    dp_mem_t        *comp_dst_sgl_vec;
    dp_mem_t        *seq_sgl_pdma;

    dp_mem_t        *comp_status_buf1;
    dp_mem_t        *comp_status_buf2;
    dp_mem_t        *comp_opaque_buf;

    // Buffers used for hash operations
    dp_mem_t        *hash_desc_vec;
    dp_mem_t        *hash_sgl_vec;

    // Since hash is last in the chain, allow the caller to supply
    // their own opaque and status buffers
    dp_mem_t        *caller_comp_pad_buf;
    dp_mem_t        *caller_hash_status_vec;
    dp_mem_t        *caller_hash_opaque_vec;
    uint32_t        caller_hash_opaque_data;

    // Hash settings
    uint8_t         sha_en;
    uint8_t         sha_type;
    uint8_t         integrity_src;
    uint8_t         integrity_type;

    // other context info
    cp_desc_t       cp_desc;
    comp_queue_t    *comp_queue;
    comp_queue_t    *hash_queue;
    comp_queue_push_t push_type;
    uint32_t        seq_comp_qid;

    uint32_t        last_cp_output_data_len;

    bool            destructor_free_buffers;
    bool            suppress_info_log;
    bool            success;
};


}  // namespace tests

#endif  // COMP_HASH_CHAIN_HPP_
