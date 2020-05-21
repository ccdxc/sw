#ifndef _CRYPTO_DRBG_HPP_
#define _CRYPTO_DRBG_HPP_

#include "logger.hpp"
#include "offload_base_params.hpp"
#include "dp_mem.hpp"
#include "nic/sdk/include/sdk/pse_intf.h"
#include "eng_if.hpp"
#include "utils.hpp"
#ifdef ELBA
#include "third-party/asic/elba/model/elb_top/elb_top_csr_defines.h"
#include "third-party/asic/elba/model/elb_top/csr_defines/elb_hens_c_hdr.h"
#else
#include "third-party/asic/capri/model/cap_top/cap_top_csr_defines.h"
#include "third-party/asic/capri/model/cap_he/readonly/cap_hens_csr_define.h"
#endif

namespace crypto_drbg {

/*
 * Barco IP code CAVS config guide specifies number of output blocks of 4
 * which for a SHA256 implementation means 4 x 32 = 128 bytes.
 */
#define CRYPTO_CTL_DRBG_OUTPUT_WDEPTH   32
#define CRYPTO_CTL_DRBG_OUTPUT_NBYTES   CRYPTO_CTL_DRBG_W2B(CRYPTO_CTL_DRBG_OUTPUT_WDEPTH)

/*
 * Some chip (ELBA) supports more than one instance
 */
typedef enum {
    DRBG_INSTANCE0,
#ifdef ELBA
    DRBG_INSTANCE1,
#endif
    DRBG_INSTANCE_MAX
} drbg_instance_t;

/*
 * HW supported instance selection
 */
static inline bool
crypto_ctl_drbg_random_num0_select(uint32_t select)
{
    return select == 0;
}

static inline bool
crypto_ctl_drbg_random_num1_select(uint32_t select)
{
    return select != 0;
}

/*
 * bytes to word, word to random (depth size), etc.
 */
#define CRYPTO_CTL_DRBG_CEIL_B2W(bytes)                                 \
    (((bytes) + sizeof(uint32_t) - 1) / sizeof(uint32_t))

#define CRYPTO_CTL_DRBG_CEIL_W2R(words, random_wdepth)                  \
    (((words) + (random_wdepth) - 1) / (random_wdepth))

#define CRYPTO_CTL_DRBG_W2B(words)                                      \
    ((words) * sizeof(uint32_t))

#define CRYPTO_CTL_DRBG_CEIL_b2B(bits)                                  \
    (((bits) + BITS_PER_BYTE - 1) / BITS_PER_BYTE)

/*
 * Register addresses
 */
#ifdef ELBA
#define DRBG_DEVICE_NAME        "Elba"

#define CRYPTO_CTL_DRBG_REG_OFFSET(inst, reg)                           \
    (ELB_ADDR_BASE_MD_HENS_OFFSET +                                     \
     ELB_HENS_CSR_DHS_CRYPTO_CTL_MD_DRBG##inst##_##reg##_BYTE_ADDRESS)

#define CRYPTO_CTL_DRBG_WORD_DEPTH(inst, reg)                           \
    ELB_HENS_CSR_DHS_CRYPTO_CTL_MD_DRBG##inst##_##reg##_ARRAY_COUNT

#else
#define DRBG_DEVICE_NAME        "Capri"

#define CRYPTO_CTL_DRBG_REG_OFFSET(inst, reg)                           \
    (CAP_ADDR_BASE_MD_HENS_OFFSET +                                     \
     CAP_HENS_CSR_DHS_CRYPTO_CTL_MD_DRBG_##reg##_BYTE_ADDRESS)

#define CRYPTO_CTL_DRBG_WORD_DEPTH(inst, reg)                           \
    CAP_HENS_CSR_DHS_CRYPTO_CTL_MD_DRBG_##reg##_ARRAY_COUNT
#endif

/*
 * Register and cryptoram access
 */
#define CRYPTO_CTL_DRBG_WR(cfg, val)                                    \
    do {                                                                \
        WRITE_REG32(regs.cfg, val);                                     \
    } while (false)

#define CRYPTO_CTL_DRBG_RD(cfg, ret_val)                                \
    do {                                                                \
        ret_val = READ_REG32(regs.cfg);                                 \
    } while (false)

/*
 * The cache should be considered as holding a big number in big-endian
 * format. The cryptoram is in little-endian format where offset 0 holds
 * the least significant word. Hence, the cache will be written in reverse
 * order from last word to first, with endian swapping.
 */
static inline void
crypto_ctl_drbg_cryptoram_wr(uint64_t ram_addr,
                             uint32_t wdepth,
                             uint32_t *wcache)
{
    wcache += wdepth - 1;
    for (uint32_t i = 0; i < wdepth; i++) {
        OFFL_LOG_DEBUG("write {:#x} @{}", htonl(*wcache),
                       CRYPTO_CTL_DRBG_W2B(i));
        WRITE_REG32(ram_addr, htonl(*wcache));
        ram_addr += sizeof(uint32_t);
        wcache--;
    }
}

/*
 * Read is handled in similar fashion as above.
 */
static inline void
crypto_ctl_drbg_cryptoram_rd(uint64_t ram_addr,
                             uint32_t wdepth,
                             uint32_t *wcache)
{
    wcache += wdepth - 1;
    for (uint32_t i = 0; i < wdepth; i++) {
        *wcache = ntohl(READ_REG32(ram_addr));
        ram_addr += sizeof(uint32_t);
        wcache--;
    }
}

/*
 * Register bit definitions
 */
#define DRBG_ISR_RNG_CPL                (1 << 0)
#define DRBG_ISR_DRNGE                  (1 << 1)

#define DRBG_MSK_RNG_INTR               (1 << 0)
#define DRBG_MSK_DRNGE_INTR             (1 << 1)

#define DRBG_ICR_EXT_INTR_EN            (1 << 0)

#define DRBG_VER_SVN_REV(v)             ((v) & 0xffff)
#define DRBG_VER_MIN_REV(v)             (((v) >> 16) & 0xff)
#define DRBG_VER_MAJ_REV(v)             (((v) >> 24) & 0xff)

#define DRBG_GCT_DRNG_PREDICT_EN        (1 << 0)

#define DRBG_GS_RNG_BUSY                (1 << 0)

#define DRBG_RNG_NDRNG_DIS              (0 << 0)        // non-DRNG disabled
#define DRBG_RNG_NDRNG_ASYNC_OSC_TGT    (1 << 0)        // non-DRNG with async oscillators (for gen on target)
#define DRBG_RNG_NDRNG_SYNC_OSC_SIM     (3 << 0)        // non-DRNG with sync oscillators (for gen in simulation)
#define DRBG_RNG_NDRNG_MASK             0x03
#define DRBG_RNG_DRNG_DIS               (0 << 2)        // generate with non-DRNG
#define DRBG_RNG_DRNG_INST              (1 << 2)        // instantiate DRBG with NDRNG and PSNL_STR_P
#define DRBG_RNG_DRNG_UNINST            (2 << 2)        // uninstantiate DRBG
#define DRBG_RNG_DRNG_INT_STATE_S       (3 << 2)        // generate with internal state S
#define DRBG_RNG_DRNG_MASK              (0x03 << 2)
#define DRBG_RNG_SIZE(v)                ((v) << 4)      // size of random number in bytes (max 512 for DRNG, 128 for NDRNG)
#define DRBG_RNG_SIZE_MASK              (0x3ff << 4)
#define DRBG_RNG_PSIZE(v)               (((v) * BITS_PER_BYTE) << 16) // size of personalization string P in bits
#define DRBG_RNG_PSIZE_MASK             (0x1ff << 16)
#define DRBG_RNG_TEST_DRNG_NDRNG_ENTROPY (0 << 29)      // use values from NDRNG entropy as input
#define DRBG_RNG_TEST_DRNG_CRYPTORAM    (1 << 29)       // use values from cryptoram as input
#define DRBG_RNG_TEST_DRNG_MASK         (1 << 29)
#define DRBG_RNG_START_SET1             (1 << 30)
#define DRBG_RNG_START_SET0             (1 << 31)
#define DRBG_RNG_START_MASK             (0x03 << 30)

#define DRBG_DRNGE_ERROR_FUNC           (1 << 0)        // catastrophic error during normal op
#define DRBG_DRNGE_ERROR_HEALTH         (1 << 1)        // error during instantiate/reseed/generate/uninstantiate
#define DRBG_DRNGE_ERROR_MASK           (0x03)

/*
 * pre push params
 */
class drbg_pre_push_params_t
{
public:

    drbg_pre_push_params_t() {}
};

/*
 * push instantiate params
 */
class drbg_push_inst_params_t
{
public:

    drbg_push_inst_params_t() :
        psnl_str_(nullptr),
        entropy_(nullptr),
        nonce_(nullptr),
        predict_resist_flag_(false)
    {
    }

    drbg_push_inst_params_t&
    psnl_str(dp_mem_t *psnl_str)
    {
        psnl_str_ = psnl_str;
        return *this;
    }
    drbg_push_inst_params_t&
    entropy(dp_mem_t *entropy)
    {
        entropy_ = entropy;
        return *this;
    }
    drbg_push_inst_params_t&
    nonce(dp_mem_t *nonce)
    {
        nonce_ = nonce;
        return *this;
    }
    drbg_push_inst_params_t&
    predict_resist_flag(bool predict_resist_flag)
    {
        predict_resist_flag_ = predict_resist_flag;
        return *this;
    }

    dp_mem_t *psnl_str(void) { return psnl_str_; }
    dp_mem_t *entropy(void) { return entropy_; }
    dp_mem_t *nonce(void) { return nonce_; }
    bool predict_resist_flag(void) { return predict_resist_flag_; }

private:
    dp_mem_t                    *psnl_str_;
    dp_mem_t                    *entropy_;
    dp_mem_t                    *nonce_;
    bool                        predict_resist_flag_;
};

/*
 * push generation params
 */
class drbg_push_gen_params_t
{
public:

    drbg_push_gen_params_t() :
        add_input_(nullptr),
        add_input_reseed_(nullptr),
        entropy_pr_(nullptr),
        entropy_reseed_(nullptr),
        ret_bits_expected_(nullptr),
        ret_bits_actual_(nullptr),
        predict_resist_req_(false),
        random_num_select_(0)
    {
    }

    drbg_push_gen_params_t&
    add_input(dp_mem_t *add_input)
    {
        add_input_ = add_input;
        return *this;
    }
    drbg_push_gen_params_t&
    add_input_reseed(dp_mem_t *add_input_reseed)
    {
        add_input_reseed_ = add_input_reseed;
        return *this;
    }
    drbg_push_gen_params_t&
    entropy_pr(dp_mem_t *entropy_pr)
    {
        entropy_pr_ = entropy_pr;
        return *this;
    }
    drbg_push_gen_params_t&
    entropy_reseed(dp_mem_t *entropy_reseed)
    {
        entropy_reseed_ = entropy_reseed;
        return *this;
    }
    drbg_push_gen_params_t&
    ret_bits_expected(dp_mem_t *ret_bits_expected)
    {
        ret_bits_expected_ = ret_bits_expected;
        return *this;
    }
    drbg_push_gen_params_t&
    ret_bits_actual(dp_mem_t *ret_bits_actual)
    {
        ret_bits_actual_ = ret_bits_actual;
        return *this;
    }
    drbg_push_gen_params_t&
    predict_resist_req(bool predict_resist_req)
    {
        predict_resist_req_ = predict_resist_req;
        return *this;
    }
    drbg_push_gen_params_t&
    random_num_select(uint32_t random_num_select)
    {
        random_num_select_ = random_num_select;
        return *this;
    }

    dp_mem_t *add_input(void) { return add_input_; }
    dp_mem_t *add_input_reseed(void) { return add_input_reseed_; }
    dp_mem_t *entropy_pr(void) { return entropy_pr_; }
    dp_mem_t *entropy_reseed(void) { return entropy_reseed_; }
    dp_mem_t *ret_bits_expected(void) { return ret_bits_expected_; }
    dp_mem_t *ret_bits_actual(void) { return ret_bits_actual_; }
    bool predict_resist_req(void) { return predict_resist_req_; }
    uint32_t random_num_select(void) { return random_num_select_; }

private:
    dp_mem_t                    *add_input_;
    dp_mem_t                    *add_input_reseed_;
    dp_mem_t                    *entropy_pr_;
    dp_mem_t                    *entropy_reseed_;
    dp_mem_t                    *ret_bits_expected_;
    dp_mem_t                    *ret_bits_actual_;
    bool                        predict_resist_req_;
    uint32_t                    random_num_select_;
};

/*
 * push uninstantiate params
 */
class drbg_push_uninst_params_t
{
public:

    drbg_push_uninst_params_t()
    {
    }
};

/*
 * push reinstantiate params, i.e.,
 * instantiate with default personalization string and entropy
 */
class drbg_push_reinst_params_t
{
public:

    drbg_push_reinst_params_t()
    {
    }
};

/*
 * Register addresses
 */
typedef struct {
    uint32_t                    psnl_wdepth;
    uint32_t                    entropy_reseed_wdepth;
    uint32_t                    entropy_inst_wdepth;
    uint32_t                    random_wdepth0;
    uint32_t                    random_wdepth1;
    uint64_t                    isr;            // interrupt status
    uint64_t                    msk;            // mask individual interrupts
    uint64_t                    icr;            // interrupt control
    uint64_t                    ver;            // version
    uint64_t                    rng;            // random number generator command
    uint64_t                    gs;             // global status
    uint64_t                    gct;            // global control
    uint64_t                    drnge;          // DRNG error status
    uint64_t                    random_num0;    // generated random number - set 0
    uint64_t                    random_num1;    // generated random number - set 1
    uint64_t                    psnl_str_p;     // personalization string P (256 bits max)
    uint64_t                    entropy_reseed; // entropy input for reseed (256 bits max)
    uint64_t                    entropy_inst;   // entropy input for instantiate (384 bits max)
} drbg_regs_t;

/*
 * Status handling
 */
class status_t
{
public:
    status_t(const drbg_regs_t& regs);
    ~status_t();

    void init(void);
    bool busy_check(void);
    bool success_check(bool failure_expected=false);

private:
    const drbg_regs_t&          regs;
    uint32_t                    gs_val;
    uint32_t                    isr_val;
    uint32_t                    drnge_val;
};

/*
 * Crypto DRBG
 */
class drbg_t {

public:
    drbg_t(drbg_instance_t inst=DRBG_INSTANCE0);
    ~drbg_t();

    bool pre_push(drbg_pre_push_params_t& pre_params);
    bool push(drbg_push_inst_params_t& inst_params);
    bool push(drbg_push_gen_params_t& gen_params);
    bool push(drbg_push_uninst_params_t& uninst_params);
    bool push(drbg_push_reinst_params_t& reinst_params);
    bool completion_check(void);
    bool full_verify(void);

    bool instantiate(void);
    bool uninstantiate(void);
    bool generate(uint8_t *buf,
                  uint32_t nbytes,
                  uint32_t random_num_select=0,
                  bool reseed=false);
    bool expected_actual_verify(const char *entity_name,
                                dp_mem_t *expected,
                                dp_mem_t *actual);
private:
    void psnl_str_set(const uint8_t *str,
                      uint32_t str_nbytes);
    void entropy_input_inst_set(const uint8_t *entropy,
                                uint32_t entropy_nbytes,
                                const uint8_t *nonce,
                                uint32_t nonce_nbytes);
    void entropy_input_reseed_set(const uint8_t *reseed,
                                  uint32_t reseed_nbytes);

    void rand_num0_get(uint8_t *buf,
                       uint32_t nbytes);
    void rand_num1_get(uint8_t *buf,
                       uint32_t nbytes);

    drbg_instance_t             inst;
    drbg_regs_t&                regs;
    drbg_push_gen_params_t      gen_params;

    uint32_t                    *psnl_str_cache;
    uint32_t                    *entropy_reseed_cache;
    uint32_t                    *entropy_inst_cache;
    uint32_t                    *rand_num0_cache;
    uint32_t                    *rand_num1_cache;
    uint32_t                    psnl_str_nbytes;

    uint32_t                    psnl_str_bdepth;
    uint32_t                    entropy_inst_bdepth;
    uint32_t                    entropy_reseed_bdepth;
    uint32_t                    nonce_bdepth;
    uint32_t                    entropy_bdepth;
    uint32_t                    random_bdepth;
    uint32_t                    rng_cfg;
    bool                        predict_resist_flag;
    status_t                    status;
};


bool init(void);
drbg_t *dflt_drbg_get(drbg_instance_t inst);

/*
 * Access methods for PSE Openssl engine
 */
extern "C" {
extern const PSE_OFFLOAD_RAND_METHOD pse_rand_offload_method;
}

} // namespace crypto_drbg

#endif   // _CRYPTO_DRBG_HPP_
