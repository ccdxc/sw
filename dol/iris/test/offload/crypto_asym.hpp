#ifndef _CRYPTO_ASYM_HPP_
#define _CRYPTO_ASYM_HPP_

#include "dp_mem.hpp"
#include "acc_ring.hpp"
#include "offload_base_params.hpp"
#include "capri_barco_crypto.hpp"
#include "capri_barco_rings.hpp"

using namespace tests;
using namespace dp_mem;

namespace crypto_asym {

/*
 * Number of paramaters fields required for HW programming:
 */
enum {
    ECDSA_NUM_PUB_KEY_FIELDS            = 2,    // Qx, Qy
    ECDSA_NUM_SIG_FIELDS                = 2,    // r, s
    ECDSA_NUM_KEY_DOMAIN_FIELDS         = 6,    // p, n, xg, yg, a, b
};

extern acc_ring_t *asym_ring;

typedef hal::pd::barco_asym_dma_descriptor_t    barco_dma_desc_t;
typedef hal::pd::capri_barco_asym_key_desc_t    barco_key_desc_t;
typedef hal::pd::barco_asym_descriptor_t        barco_req_desc_t;

typedef union {
    uint32_t            full_word;

    uint32_t            fail_addr   : 4,
                        err_flags   : 12,
                        pk_busy     : 1,
                        intr_status : 1,
                        unused      : 14;
} __attribute__((__packed__)) barco_asym_status_t;

#define CRYPTO_ASYM_DMA_DESC_ALIGNMENT          \
        (sizeof(barco_dma_desc_t) < 32 ? 32 : sizeof(barco_dma_desc_t))
#define CRYPTO_ASYM_KEY_DESC_ALIGNMENT          sizeof(crypto_asym::key_desc_t)
#define CRYPTO_ASYM_REQ_DESC_ALIGNMENT          sizeof(crypto_asym::req_desc_t)

/*
 * full word status_t bits
 */
#define CRYPTO_ASYM_STATUS_PPX_NOT_ON_CURVE     (1 << 4)
#define CRYPTO_ASYM_STATUS_PPX_AT_INFINITE      (1 << 5)
#define CRYPTO_ASYM_STATUS_COUPLE_NOT_VALID     (1 << 6)
#define CRYPTO_ASYM_STATUS_PARAM_N_NOT_VALID    (1 << 7)
#define CRYPTO_ASYM_STATUS_NOT_IMPLEMENTED      (1 << 8)
#define CRYPTO_ASYM_STATUS_SIG_NOT_VALID        (1 << 9)
#define CRYPTO_ASYM_STATUS_PARAM_AB_NOT_VALID   (1 << 10)
#define CRYPTO_ASYM_STATUS_NOT_INVERTIBLE       (1 << 11)
#define CRYPTO_ASYM_STATUS_COMPOSITE            (1 << 12)
#define CRYPTO_ASYM_STATUS_NOT_QUAD_RESIDUE     (1 << 13)
#define CRYPTO_ASYM_STATUS_UNUSED0              (1 << 14)
#define CRYPTO_ASYM_STATUS_UNUSED1              (1 << 15)
#define CRYPTO_ASYM_STATUS_PK_BUSY              (1 << 16)
#define CRYPTO_ASYM_STATUS_INTR                 (1 << 17)

class status_t
{
public:
    status_t(dp_mem_type_t mem_type);
    ~status_t();

    void init(void);
    bool busy_check(void);
    bool success_check(bool failure_expected=false);
    uint64_t pa(void) { return asym_status->pa(); }

private:
    dp_mem_t                    *asym_status;
};

/*
 * SW key index type (negative means invalid)
 */
#define CRYPTO_ASYM_KEY_IDX_INVALID             -1
typedef int32_t key_idx_t;

static inline bool
key_idx_is_valid(key_idx_t key_idx)
{
    return key_idx >= 0;
}

/*
 * DMA descriptor index, max value is unrestricted
 */
#define CRYPTO_ASYM_DMA_DESC_IDX_INVALID -1
typedef int     dma_desc_idx_t;

static inline bool
dma_desc_idx_is_valid(dma_desc_idx_t desc_idx)
{
    return desc_idx >= 0;
}

/*
 * Emulate named parameters support for dma_desc_pool_t constructor
 */
class dma_desc_pool_params_t
{
public:

    dma_desc_pool_params_t() :
        dma_desc_mem_type_(DP_MEM_TYPE_HBM),
        num_descs_(0)
    {
    }

    dma_desc_pool_params_t&
    dma_desc_mem_type(dp_mem_type_t dma_desc_mem_type)
    {
        dma_desc_mem_type_ = dma_desc_mem_type;
        return *this;
    }
    dma_desc_pool_params_t&
    num_descs(uint32_t num_descs)
    {
        num_descs_ = num_descs;
        return *this;
    }
    dma_desc_pool_params_t&
    base_params(offload_base_params_t& base_params)
    {
        base_params_ = base_params;
        return *this;
    }

    dp_mem_type_t dma_desc_mem_type(void) { return dma_desc_mem_type_; }
    uint32_t num_descs(void) { return num_descs_; }
    offload_base_params_t& base_params(void) { return base_params_; }

private:
    dp_mem_type_t               dma_desc_mem_type_;
    uint32_t                    num_descs_;
    offload_base_params_t       base_params_;
};


/*
 * Emulate named parameters support for dma_desc_pool_t pre_push
 */
class dma_desc_pool_pre_push_params_t
{
public:
    dma_desc_pool_pre_push_params_t() :
         desc_idx_(CRYPTO_ASYM_DMA_DESC_IDX_INVALID),
         next_idx_(CRYPTO_ASYM_DMA_DESC_IDX_INVALID),
         data_(nullptr),
         double_size_(false)
    {
    }

    dma_desc_pool_pre_push_params_t&
    desc_idx(dma_desc_idx_t desc_idx)
    {
        desc_idx_ = desc_idx;
        return *this;
    }
    dma_desc_pool_pre_push_params_t&
    next_idx(dma_desc_idx_t next_idx)
    {
        next_idx_ = next_idx;
        return *this;
    }
    dma_desc_pool_pre_push_params_t&
    data(dp_mem_t *data)
    {
        data_ = data;
        return *this;
    }
    dma_desc_pool_pre_push_params_t&
    double_size(bool double_size)
    {
        double_size_ = double_size;
        return *this;
    }

    dma_desc_idx_t desc_idx(void) { return desc_idx_; }
    dma_desc_idx_t next_idx(void) { return next_idx_; }
    dp_mem_t *data(void) { return data_; }
    bool double_size(void) { return double_size_; }

private:
    dma_desc_idx_t              desc_idx_;
    dma_desc_idx_t              next_idx_;
    dp_mem_t                    *data_;
    bool                        double_size_;
};

/*
 * DMA descriptor pool
 */
class dma_desc_pool_t {

public:
    dma_desc_pool_t(dma_desc_pool_params_t& params);
    ~dma_desc_pool_t();

    uint64_t pre_push(dma_desc_pool_pre_push_params_t& pre_params);
    bool push(void);

private:

    dma_desc_pool_params_t      params;
    dp_mem_t                    *dma_desc_vec;

    bool                        hw_started;
    bool                        test_success;
};


/*
 * Emulate named parameters support for req_desc_t pre_push
 */
class req_desc_pre_push_params_t
{
public:

    req_desc_pre_push_params_t() :
         input_list_addr_(0),
         output_list_addr_(0),
         status_addr_(0),
         key_idx_(CRYPTO_ASYM_KEY_IDX_INVALID),
         opaque_tag_value_(0),
         opaque_tag_en_(false)
    {
    }

    req_desc_pre_push_params_t&
    input_list_addr(uint64_t input_list_addr)
    {
        input_list_addr_ = input_list_addr;
        return *this;
    }
    req_desc_pre_push_params_t&
    output_list_addr(uint64_t output_list_addr)
    {
        output_list_addr_ = output_list_addr;
        return *this;
    }
    req_desc_pre_push_params_t&
    status_addr(uint64_t status_addr)
    {
        status_addr_ = status_addr;
        return *this;
    }
    req_desc_pre_push_params_t&
    key_idx(uint32_t key_idx)
    {
        key_idx_ = key_idx;
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

    uint64_t input_list_addr(void) { return input_list_addr_; }
    uint64_t output_list_addr(void) { return output_list_addr_; }
    uint64_t status_addr(void) { return status_addr_; }
    uint32_t key_idx(void) { return key_idx_; }
    uint32_t opaque_tag_value(void) { return opaque_tag_value_; }
    bool opaque_tag_en(void) { return opaque_tag_en_; }

private:
    uint64_t                    input_list_addr_;
    uint64_t                    output_list_addr_;
    uint64_t                    status_addr_;
    uint32_t                    key_idx_;
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

/*
 * Emulate named parameters support for key_desc_t pre_push
 */
class key_desc_pre_push_params_t
{
public:

    key_desc_pre_push_params_t() :
         key_param_list_(0),
         cmd_size_(0),
         swap_bytes_(false),
         cmd_rsa_sign_(false),
         cmd_rsa_verify_(false),
         cmd_rsa_encrypt_(false),
         cmd_rsa_decrypt_(false),
         cmd_ecdsa_sign_(false),
         cmd_ecdsa_verify_(false)
    {
    }

    key_desc_pre_push_params_t&
    key_param_list(uint64_t key_param_list)
    {
        key_param_list_ = key_param_list;
        return *this;
    }
    key_desc_pre_push_params_t&
    cmd_size(uint32_t cmd_size)
    {
        cmd_size_ = cmd_size;
        return *this;
    }
    key_desc_pre_push_params_t&
    swap_bytes(bool swap_bytes)
    {
        swap_bytes_ = swap_bytes;
        return *this;
    }
    key_desc_pre_push_params_t&
    cmd_rsa_sign(bool cmd_rsa_sign)
    {
        cmd_rsa_sign_ = cmd_rsa_sign;
        return *this;
    }
    key_desc_pre_push_params_t&
    cmd_rsa_verify(bool cmd_rsa_verify)
    {
        cmd_rsa_verify_ = cmd_rsa_verify;
        return *this;
    }
    key_desc_pre_push_params_t&
    cmd_rsa_encrypt(bool cmd_rsa_encrypt)
    {
        cmd_rsa_encrypt_ = cmd_rsa_encrypt;
        return *this;
    }
    key_desc_pre_push_params_t&
    cmd_rsa_decrypt(bool cmd_rsa_decrypt)
    {
        cmd_rsa_decrypt_ = cmd_rsa_decrypt;
        return *this;
    }
    key_desc_pre_push_params_t&
    cmd_ecdsa_sign(bool cmd_ecdsa_sign)
    {
        cmd_ecdsa_sign_ = cmd_ecdsa_sign;
        return *this;
    }
    key_desc_pre_push_params_t&
    cmd_ecdsa_verify(bool cmd_ecdsa_verify)
    {
        cmd_ecdsa_verify_ = cmd_ecdsa_verify;
        return *this;
    }

    uint64_t key_param_list(void) { return key_param_list_; }
    uint32_t cmd_size(void) { return cmd_size_; }
    bool swap_bytes(void) { return swap_bytes_; }
    bool cmd_rsa_sign(void) { return cmd_rsa_sign_; }
    bool cmd_rsa_verify(void) { return cmd_rsa_verify_; }
    bool cmd_rsa_encrypt(void) { return cmd_rsa_encrypt_; }
    bool cmd_rsa_decrypt(void) { return cmd_rsa_decrypt_; }
    bool cmd_ecdsa_sign(void) { return cmd_ecdsa_sign_; }
    bool cmd_ecdsa_verify(void) { return cmd_ecdsa_verify_; }

private:
    uint64_t                    key_param_list_;
    uint32_t                    cmd_size_;
    bool                        swap_bytes_;
    bool                        cmd_rsa_sign_;
    bool                        cmd_rsa_verify_;
    bool                        cmd_rsa_encrypt_;
    bool                        cmd_rsa_decrypt_;
    bool                        cmd_ecdsa_sign_;
    bool                        cmd_ecdsa_verify_;
};

/*
 * Key descriptor
 */
class key_desc_t {

public:
    bool pre_push(key_desc_pre_push_params_t& pre_params);
    void *push(void);

    uint32_t size_get(void) { return sizeof(key_desc); }

private:
    barco_key_desc_t            key_desc;
};


bool init(const char *engine_path);

} // namespace crypto_asym

#endif   // _CRYPTO_ASYM_HPP_
