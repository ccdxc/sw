#ifndef _CRYPTO_SYMM_HPP_
#define _CRYPTO_SYMM_HPP_

#include "dp_mem.hpp"
#include "acc_ring.hpp"
#include "offload_base_params.hpp"
#include "capri_barco_crypto.hpp"
#include "capri_barco_rings.hpp"

using namespace tests;
using namespace dp_mem;
using namespace std;

namespace crypto_symm {

/*
 * Crypto types
 */
typedef enum {
    CRYPTO_SYMM_TYPE_SHA,
    CRYPTO_SYMM_TYPE_SHA3,
    CRYPTO_SYMM_TYPE_AES_CBC,
    CRYPTO_SYMM_TYPE_AES_CCM,
    CRYPTO_SYMM_TYPE_AES_GCM,
    CRYPTO_SYMM_TYPE_AES_XTS,
    CRYPTO_SYMM_TYPE_ECB,
    CRYPTO_SYMM_TYPE_CBC,
} crypto_symm_type_t;

typedef enum {
    CRYPTO_SYMM_OP_GENERATE,
    CRYPTO_SYMM_OP_VERIFY,
    CRYPTO_SYMM_OP_ENCRYPT,
    CRYPTO_SYMM_OP_DECRYPT,
} crypto_symm_op_t;

extern acc_ring_t *mpp0_ring;

typedef sdk::platform::capri::barco_sym_msg_descriptor_t     barco_msg_desc_t;
typedef sdk::platform::capri::barco_symm_req_descriptor_t    barco_req_desc_t;

typedef struct {
    uint32_t            full_word;
} __attribute__((__packed__)) barco_symm_status_t;

typedef struct {
    uint64_t            data;
} __attribute__((__packed__)) barco_symm_doorbell_t;

#define CRYPTO_SYMM_MSG_DESC_ALIGNMENT          sizeof(crypto_symm::barco_msg_desc_t)
#define CRYPTO_SYMM_REQ_DESC_ALIGNMENT          sizeof(crypto_symm::barco_req_desc_t)
#define CRYPTO_SYMM_STATUS_ALIGNMENT            sizeof(uint64_t)
#define CRYPTO_SYMM_DOORBELL_ALIGNMENT          sizeof(barco_symm_doorbell_t)
#define CRYPTO_SYMM_KEY_ALIGNMENT               sizeof(uint64_t)
#define CRYPTO_SYMM_IV_ALIGNMENT                sizeof(uint64_t)
#define CRYPTO_SYMM_MSG_INPUT_ALIGNMENT         sizeof(uint64_t)
#define CRYPTO_SYMM_HASH_OUTPUT_ALIGNMENT       sizeof(uint64_t)

/*
 * full word status_t bits
 */
#define CRYPTO_SYMM_T10_WRONG_PROT_INFO         (1 << 0)
#define CRYPTO_SYMM_LEN_NOT_MULTIPLE            (1 << 1)

class status_t
{
public:
    status_t(dp_mem_type_t mem_type);
    ~status_t();

    void init(void);
    bool busy_check(void);
    bool success_check(bool failure_expected=false);
    uint64_t pa(void) { return symm_status->pa(); }

private:
    dp_mem_t                    *symm_status;
};

class doorbell_t
{
public:
    doorbell_t(dp_mem_type_t mem_type,
               uint64_t data=0xaecbecaaaecbecaa);
    ~doorbell_t();

    void init(void);
    bool busy_check(void);
    bool success_check(bool failure_expected=false);
    uint64_t pa(void) { return symm_doorbell->pa(); }
    uint64_t data(void) { return data_; }

private:
    dp_mem_t                    *symm_doorbell;
    uint64_t                    data_;
};

/*
 * SW key index type (negative means invalid)
 */
#define CRYPTO_SYMM_KEY_IDX_INVALID             -1
typedef int32_t key_idx_t;

static inline bool
key_idx_is_valid(key_idx_t key_idx)
{
    return key_idx >= 0;
}

/*
 * MSG descriptor index, max value is unrestricted
 */
#define CRYPTO_SYMM_MSG_DESC_IDX_INVALID -1
typedef int     msg_desc_idx_t;

static inline bool
msg_desc_idx_is_valid(msg_desc_idx_t desc_idx)
{
    return desc_idx >= 0;
}

/*
 * Emulate named parameters support for msg_desc_pool_t constructor
 */
class msg_desc_pool_params_t
{
public:

    msg_desc_pool_params_t() :
        msg_desc_mem_type_(DP_MEM_TYPE_HBM),
        num_descs_(0)
    {
    }

    msg_desc_pool_params_t&
    msg_desc_mem_type(dp_mem_type_t msg_desc_mem_type)
    {
        msg_desc_mem_type_ = msg_desc_mem_type;
        return *this;
    }
    msg_desc_pool_params_t&
    num_descs(uint32_t num_descs)
    {
        num_descs_ = num_descs;
        return *this;
    }
    msg_desc_pool_params_t&
    base_params(offload_base_params_t& base_params)
    {
        base_params_ = base_params;
        return *this;
    }

    dp_mem_type_t msg_desc_mem_type(void) { return msg_desc_mem_type_; }
    uint32_t num_descs(void) { return num_descs_; }
    offload_base_params_t& base_params(void) { return base_params_; }

private:
    dp_mem_type_t               msg_desc_mem_type_;
    uint32_t                    num_descs_;
    offload_base_params_t       base_params_;
};


/*
 * Emulate named parameters support for msg_desc_pool_t pre_push
 */
class msg_desc_pool_pre_push_params_t
{
public:
    msg_desc_pool_pre_push_params_t(vector<dp_mem_t *>& msg_vec) :
         msg_vec_(msg_vec)
    {
    }

    vector<dp_mem_t *>& msg_vec(void) { return msg_vec_; }
  
private:
    vector<dp_mem_t *>&         msg_vec_;
};

/*
 * MSG descriptor pool
 */
class msg_desc_pool_t {

public:
    msg_desc_pool_t(msg_desc_pool_params_t& params);
    ~msg_desc_pool_t();

    uint64_t pre_push(msg_desc_pool_pre_push_params_t& pre_params);
    bool push(void);

private:

    msg_desc_pool_params_t      params;
    dp_mem_t                    *msg_desc_vec;

    bool                        hw_started;
    bool                        test_success;
};

/*
 * Emulate named parameters support for cmd_t pre_push
 */
class cmd_pre_push_params_t
{
public:

    cmd_pre_push_params_t() :
         type_(CRYPTO_SYMM_TYPE_SHA),
         op_(CRYPTO_SYMM_OP_GENERATE),
         sha_nbytes_(0)
    {
    }

    cmd_pre_push_params_t&
    type(crypto_symm_type_t type)
    {
        type_ = type;
        return *this;
    }
    cmd_pre_push_params_t&
    op(crypto_symm_op_t op)
    {
        op_ = op;
        return *this;
    }
    cmd_pre_push_params_t&
    sha_nbytes(uint32_t sha_nbytes)
    {
        sha_nbytes_ = sha_nbytes;
        return *this;
    }

    crypto_symm_type_t type(void) { return type_; }
    crypto_symm_op_t op(void) { return op_; }
    uint32_t sha_nbytes(void) { return sha_nbytes_; }
    uint32_t cmd_eval(void);

private:
    crypto_symm_type_t          type_;
    crypto_symm_op_t            op_;
    uint32_t                    sha_nbytes_;
};

/*
 * Emulate named parameters support for req_desc_t pre_push
 */
class req_desc_pre_push_params_t
{
public:

    req_desc_pre_push_params_t() :
         src_msg_desc_addr_(0),
         dst_msg_desc_addr_(0),
         iv_addr_(0),
         auth_tag_addr_(0),
         status_addr_(0),
         db_addr_(0),
         db_data_(0),
         key_idx0_(CRYPTO_SYMM_KEY_IDX_INVALID),
         key_idx1_(CRYPTO_SYMM_KEY_IDX_INVALID),
         header_size_(0),
         sector_size_(0),
         sector_num_(0),
         app_tag_(0),
         opaque_tag_value_(0),
         opaque_tag_en_(false)
    {
    }

    req_desc_pre_push_params_t&
    cmd_pre_push(cmd_pre_push_params_t& cmd_pre_push)
    {
        cmd_pre_push_ = cmd_pre_push;
        return *this;
    }
    req_desc_pre_push_params_t&
    src_msg_desc_addr(uint64_t src_msg_desc_addr)
    {
        src_msg_desc_addr_ = src_msg_desc_addr;
        return *this;
    }
    req_desc_pre_push_params_t&
    dst_msg_desc_addr(uint64_t dst_msg_desc_addr)
    {
        dst_msg_desc_addr_ = dst_msg_desc_addr;
        return *this;
    }
    req_desc_pre_push_params_t&
    iv_addr(uint64_t iv_addr)
    {
        iv_addr_ = iv_addr;
        return *this;
    }
    req_desc_pre_push_params_t&
    auth_tag_addr(uint64_t auth_tag_addr)
    {
        auth_tag_addr_ = auth_tag_addr;
        return *this;
    }
    req_desc_pre_push_params_t&
    status_addr(uint64_t status_addr)
    {
        status_addr_ = status_addr;
        return *this;
    }
    req_desc_pre_push_params_t&
    db_addr(uint64_t db_addr)
    {
        db_addr_ = db_addr;
        return *this;
    }
    req_desc_pre_push_params_t&
    db_data(uint64_t db_data)
    {
        db_data_ = db_data;
        return *this;
    }
    req_desc_pre_push_params_t&
    key_idx0(key_idx_t key_idx0)
    {
        key_idx0_ = key_idx0;
        return *this;
    }
    req_desc_pre_push_params_t&
    key_idx1(key_idx_t key_idx1)
    {
        key_idx1_ = key_idx1;
        return *this;
    }
    req_desc_pre_push_params_t&
    header_size(uint32_t header_size)
    {
        header_size_ = header_size;
        return *this;
    }
    req_desc_pre_push_params_t&
    sector_size(uint32_t sector_size)
    {
        sector_size_ = sector_size;
        return *this;
    }
    req_desc_pre_push_params_t&
    sector_num(uint32_t sector_num)
    {
        sector_num_ = sector_num;
        return *this;
    }
    req_desc_pre_push_params_t&
    app_tag(uint32_t app_tag)
    {
        app_tag_ = app_tag;
        return *this;
    }
    req_desc_pre_push_params_t&
    opaque_tag_value(uint32_t opaque_tag_value)
    {
        opaque_tag_value_ = opaque_tag_value;
        return *this;
    }
    req_desc_pre_push_params_t&
    opaque_tag_en(bool opaque_tag_en)
    {
        opaque_tag_en_ = opaque_tag_en;
        return *this;
    }

    cmd_pre_push_params_t& cmd_pre_push(void) { return cmd_pre_push_; }
    uint64_t src_msg_desc_addr(void) { return src_msg_desc_addr_; }
    uint64_t dst_msg_desc_addr(void) { return dst_msg_desc_addr_; }
    uint64_t iv_addr(void) { return iv_addr_; }
    uint64_t auth_tag_addr(void) { return auth_tag_addr_; }
    uint64_t status_addr(void) { return status_addr_; }
    uint64_t db_addr(void) { return db_addr_; }
    uint64_t db_data(void) { return db_data_; }
    uint32_t key_idx0(void) { return key_idx0_; }
    uint32_t key_idx1(void) { return key_idx1_; }
    uint32_t header_size(void) { return header_size_; }
    uint32_t sector_size(void) { return sector_size_; }
    uint32_t sector_num(void) { return sector_num_; }
    uint32_t app_tag(void) { return app_tag_; }
    uint32_t opaque_tag_value(void) { return opaque_tag_value_; }
    bool opaque_tag_en(void) { return opaque_tag_en_; }

private:
    cmd_pre_push_params_t       cmd_pre_push_;
    uint64_t                    src_msg_desc_addr_;
    uint64_t                    dst_msg_desc_addr_;
    uint64_t                    iv_addr_;
    uint64_t                    auth_tag_addr_;
    uint64_t                    status_addr_;
    uint64_t                    db_addr_;
    uint64_t                    db_data_;
    key_idx_t                   key_idx0_;
    key_idx_t                   key_idx1_;
    uint32_t                    header_size_;
    uint32_t                    sector_size_;
    uint32_t                    sector_num_;
    uint32_t                    app_tag_;
    uint32_t                    opaque_tag_value_;
    bool                        opaque_tag_en_;
};

/*
 * Request descriptor
 */
class req_desc_t {

public:
    bool pre_push(req_desc_pre_push_params_t& pre_params);
    void *push(void);

private:
    barco_req_desc_t            req_desc;
};


bool init(const char *engine_path);

} // namespace crypto_symm

#endif   // _CRYPTO_SYMM_HPP_
