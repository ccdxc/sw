#ifndef	CHKSUM_DECOMP_CHAIN_HPP_
#define	CHKSUM_DECOMP_CHAIN_HPP_

#include "tests.hpp"
#include "compression.hpp"
#include "comp_hash_chain.hpp"

#include <vector>

namespace tests {

/*
 * Emulate named parameters support for chksum_decomp_chain_t constructor
 */
class chksum_decomp_chain_params_t
{
public:

    chksum_decomp_chain_params_t() :
        app_max_size_(0),
        uncomp_mem_type_(DP_MEM_TYPE_VOID),
        destructor_free_buffers_(false),
        suppress_info_log_(false)
    {
    }

    uint32_t        app_max_size_;
    dp_mem_type_t   uncomp_mem_type_;
    bool            destructor_free_buffers_;
    bool            suppress_info_log_;

    chksum_decomp_chain_params_t&
    app_max_size(uint32_t app_max_size)
    {
        app_max_size_ = app_max_size;
        return *this;
    }
    chksum_decomp_chain_params_t&
    uncomp_mem_type(dp_mem_type_t uncomp_mem_type)
    {
        uncomp_mem_type_ = uncomp_mem_type;
        return *this;
    }
    chksum_decomp_chain_params_t&
    destructor_free_buffers(bool destructor_free_buffers)
    {
        destructor_free_buffers_ = destructor_free_buffers;
        return *this;
    }
    chksum_decomp_chain_params_t&
    suppress_info_log(bool suppress_info_log)
    {
        suppress_info_log_ = suppress_info_log;
        return *this;
    }
};


/*
 * Emulate named parameters support for chksum_decomp_chain_t pre_push
 */
class chksum_decomp_chain_pre_push_params_t
{
public:

    chksum_decomp_chain_pre_push_params_t() :
        caller_chksum_status_vec_(nullptr),
        caller_chksum_opaque_vec_(nullptr),
        caller_decomp_status_buf_(nullptr),
        caller_decomp_opaque_buf_(nullptr),
        caller_opaque_data_(0)
    {
    }

    dp_mem_t    *caller_chksum_status_vec_;
    dp_mem_t    *caller_chksum_opaque_vec_;
    dp_mem_t    *caller_decomp_status_buf_;
    dp_mem_t    *caller_decomp_opaque_buf_;
    uint32_t    caller_opaque_data_;

    chksum_decomp_chain_pre_push_params_t&
    caller_chksum_status_vec(dp_mem_t *caller_chksum_status_vec)
    {
        caller_chksum_status_vec_ = caller_chksum_status_vec;
        return *this;
    }
    chksum_decomp_chain_pre_push_params_t&
    caller_chksum_opaque_vec(dp_mem_t *caller_chksum_opaque_vec)
    {
        caller_chksum_opaque_vec_ = caller_chksum_opaque_vec;
        return *this;
    }
    chksum_decomp_chain_pre_push_params_t&
    caller_decomp_status_buf(dp_mem_t *caller_decomp_status_buf)
    {
        caller_decomp_status_buf_ = caller_decomp_status_buf;
        return *this;
    }
    chksum_decomp_chain_pre_push_params_t&
    caller_decomp_opaque_buf(dp_mem_t *caller_decomp_opaque_buf)
    {
        caller_decomp_opaque_buf_ = caller_decomp_opaque_buf;
        return *this;
    }
    chksum_decomp_chain_pre_push_params_t&
    caller_opaque_data(uint32_t caller_opaque_data)
    {
        caller_opaque_data_ = caller_opaque_data;
        return *this;
    }
};


/*
 * Emulate named parameters support for chksum_decomp_chain_t push
 */
class chksum_decomp_chain_push_params_t
{
public:

    chksum_decomp_chain_push_params_t() :
        comp_hash_chain_(nullptr),
        chksum_queue_(nullptr),
        decomp_queue_(nullptr),
        push_type_(COMP_QUEUE_PUSH_SEQUENCER),
        seq_chksum_qid_(0),
        seq_decomp_qid_(0)
    {
    }

    comp_hash_chain_t   *comp_hash_chain_;
    comp_queue_t        *chksum_queue_;
    comp_queue_t        *decomp_queue_;
    comp_queue_push_t   push_type_;
    uint32_t            seq_chksum_qid_;
    uint32_t            seq_decomp_qid_;

    chksum_decomp_chain_push_params_t&
    comp_hash_chain(comp_hash_chain_t *comp_hash_chain)
    {
        comp_hash_chain_ = comp_hash_chain;
        return *this;
    }
    chksum_decomp_chain_push_params_t&
    chksum_queue(comp_queue_t *chksum_queue)
    {
        chksum_queue_ = chksum_queue;
        return *this;
    }
    chksum_decomp_chain_push_params_t&
    decomp_queue(comp_queue_t *decomp_queue)
    {
        decomp_queue_ = decomp_queue;
        return *this;
    }
    chksum_decomp_chain_push_params_t&
    push_type(comp_queue_push_t push_type)
    {
        push_type_ = push_type;
        return *this;
    }
    chksum_decomp_chain_push_params_t&
    seq_chksum_qid(uint32_t seq_chksum_qid)
    {
        seq_chksum_qid_ = seq_chksum_qid;
        return *this;
    }
    chksum_decomp_chain_push_params_t&
    seq_decomp_qid(uint32_t seq_decomp_qid)
    {
        seq_decomp_qid_ = seq_decomp_qid;
        return *this;
    }
};

/*
 * Accelerator checksum to decompression chaining DOL
 */
class chksum_decomp_chain_t
{
public:

    chksum_decomp_chain_t(chksum_decomp_chain_params_t params);
    ~chksum_decomp_chain_t();

    void pre_push(chksum_decomp_chain_pre_push_params_t params);
    int  push(chksum_decomp_chain_push_params_t params);
    void post_push(void);
    int  fast_verify(void);
    int  full_verify(void);

private:

    cp_desc_t& chksum_setup(uint32_t block_no);
    cp_desc_t& decomp_setup(void);

    comp_hash_chain_t   *comp_hash_chain;
    uint32_t            app_max_size;
    uint32_t            app_blk_size;
    uint32_t            app_hash_size;
    uint32_t            max_hash_blks;
    uint32_t            num_hash_blks;

    // Buffers used for decompression operations
    dp_mem_t            *uncomp_buf;
    dp_mem_t            *caller_comp_buf;

    // Buffers used for checksum operations
    dp_mem_t            *chksum_status_vec;

    // Since decomp is last in the chain, allow the caller to supply
    // their own opaque and status buffers
    dp_mem_t            *caller_chksum_status_vec;
    dp_mem_t            *caller_chksum_opaque_vec;
    dp_mem_t            *caller_decomp_status_buf;
    dp_mem_t            *caller_decomp_opaque_buf;
    uint32_t            caller_opaque_data;

    // other context info
    std::vector<cp_desc_t>  chksum_desc_vec;
    cp_desc_t           dc_desc;

    comp_queue_t        *chksum_queue;
    comp_queue_t        *decomp_queue;
    comp_queue_push_t   chksum_push_type;
    comp_queue_push_t   decomp_push_type;
    uint32_t            seq_chksum_qid;
    uint32_t            seq_decomp_qid;

    bool                destructor_free_buffers;
    bool                suppress_info_log;
    bool                success;
};


}  // namespace tests

#endif  // CHKSUM_DECOMP_CHAIN_HPP_
