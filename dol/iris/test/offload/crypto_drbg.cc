#include "crypto_drbg.hpp"

/*
 * Deterministic Random Bit Generator
 */
namespace crypto_drbg {

static const string dflt_psnl_str("Pensando " DRBG_DEVICE_NAME " psnl DRBG");
static const string dflt_entropy("Pensando " DRBG_DEVICE_NAME " entropy DRBG");

#define DRBG_REGS_DEF(inst)                                                             \
    .psnl_wdepth           = CRYPTO_CTL_DRBG_WORD_DEPTH(inst, CRYPTORAM_PSNL_STR_P),    \
    .entropy_reseed_wdepth = CRYPTO_CTL_DRBG_WORD_DEPTH(inst, CRYPTORAM_ENTROPY_RESEED),\
    .entropy_inst_wdepth   = CRYPTO_CTL_DRBG_WORD_DEPTH(inst, CRYPTORAM_ENTROPY_INST),  \
    .random_wdepth0        = CRYPTO_CTL_DRBG_WORD_DEPTH(inst, CRYPTORAM_RANDOM_NUM0),   \
    .random_wdepth1        = CRYPTO_CTL_DRBG_WORD_DEPTH(inst, CRYPTORAM_RANDOM_NUM1),   \
    .isr                   = CRYPTO_CTL_DRBG_REG_OFFSET(inst, ISR),                     \
    .msk                   = CRYPTO_CTL_DRBG_REG_OFFSET(inst, MSK),                     \
    .icr                   = CRYPTO_CTL_DRBG_REG_OFFSET(inst, ICR),                     \
    .ver                   = CRYPTO_CTL_DRBG_REG_OFFSET(inst, VER),                     \
    .rng                   = CRYPTO_CTL_DRBG_REG_OFFSET(inst, RNG),                     \
    .gs                    = CRYPTO_CTL_DRBG_REG_OFFSET(inst, GS),                      \
    .gct                   = CRYPTO_CTL_DRBG_REG_OFFSET(inst, GCT),                     \
    .drnge                 = CRYPTO_CTL_DRBG_REG_OFFSET(inst, DRNGE),                   \
    .random_num0           = CRYPTO_CTL_DRBG_REG_OFFSET(inst, CRYPTORAM_RANDOM_NUM0),   \
    .random_num1           = CRYPTO_CTL_DRBG_REG_OFFSET(inst, CRYPTORAM_RANDOM_NUM1),   \
    .psnl_str_p            = CRYPTO_CTL_DRBG_REG_OFFSET(inst, CRYPTORAM_PSNL_STR_P),    \
    .entropy_reseed        = CRYPTO_CTL_DRBG_REG_OFFSET(inst, CRYPTORAM_ENTROPY_RESEED),\
    .entropy_inst          = CRYPTO_CTL_DRBG_REG_OFFSET(inst, CRYPTORAM_ENTROPY_INST),  \


static const drbg_regs_t drbg_regs_tbl[DRBG_INSTANCE_MAX] = {
    {
        DRBG_REGS_DEF(0)
    },
#ifdef ELBA
    {
        DRBG_REGS_DEF(1)
    },
#endif
};

static drbg_t           *dflt_drbg_tbl[DRBG_INSTANCE_MAX];

bool
init(void)
{
    /*
     * DRBG blocks already initialized by HAL;
     * create global default drbg for convenience.
     */
    for (int inst = DRBG_INSTANCE0; inst < DRBG_INSTANCE_MAX; inst++) {
        dflt_drbg_tbl[inst] = new drbg_t((drbg_instance_t)inst);
    }
    return true;
}

drbg_t *
dflt_drbg_get(drbg_instance_t inst)
{
    return inst < DRBG_INSTANCE_MAX ? dflt_drbg_tbl[inst] : nullptr;
}


drbg_t::drbg_t(drbg_instance_t inst) :
    inst(inst),
    regs(inst < DRBG_INSTANCE_MAX ? drbg_regs_tbl[inst] : drbg_regs_tbl[DRBG_INSTANCE0]),
    predict_resist_flag(false),
    status(regs)
{
    drbg_push_reinst_params_t   reinst_params;
    uint32_t                    vers;

    assert(inst < DRBG_INSTANCE_MAX);
    CRYPTO_CTL_DRBG_RD(ver, vers);
    OFFL_FUNC_INFO("instance {} svn {:#x} major {:#x} minor {:#x}",
                   inst, DRBG_VER_SVN_REV(vers),
                   DRBG_VER_MAJ_REV(vers), DRBG_VER_MIN_REV(vers));
    OFFL_FUNC_INFO("psnl_wdepth {} entropy_reseed_wdepth {} entropy_inst_wdepth {} "
                   "random_wdepth0 {} random_wdepth1 {} ", regs.psnl_wdepth,
                   regs.entropy_reseed_wdepth, regs.entropy_inst_wdepth,
                   regs.random_wdepth0, regs.random_wdepth1);
    assert(regs.random_wdepth0 && (regs.random_wdepth0 == regs.random_wdepth1));
    random_bdepth = CRYPTO_CTL_DRBG_OUTPUT_NBYTES;
    rng_cfg = DRBG_RNG_SIZE(random_bdepth);

    psnl_str_cache       = new uint32_t[regs.psnl_wdepth];
    entropy_reseed_cache = new uint32_t[regs.entropy_reseed_wdepth];
    entropy_inst_cache   = new uint32_t[regs.entropy_inst_wdepth];
    rand_num0_cache      = new uint32_t[regs.random_wdepth0];
    rand_num1_cache      = new uint32_t[regs.random_wdepth1];

    if (!push(reinst_params)) {
        OFFL_FUNC_ERR("instance {} failed instantiation", inst);
    }
}

drbg_t::~drbg_t()
{
    delete [] psnl_str_cache;
    delete [] entropy_reseed_cache;
    delete [] entropy_inst_cache;
    delete [] rand_num0_cache;
    delete [] rand_num1_cache;
}

/*
 */
bool
drbg_t::pre_push(drbg_pre_push_params_t& pre_params)
{
    return true;
}


/*
 * Instantiate operation
 */
bool
drbg_t::push(drbg_push_inst_params_t& inst_params)
{
    predict_resist_flag = inst_params.predict_resist_flag();
    psnl_str_set(inst_params.psnl_str()->read(),
                 inst_params.psnl_str()->content_size_get());
    entropy_input_inst_set(inst_params.entropy()->read(),
                           inst_params.entropy()->content_size_get(),
                           inst_params.nonce()->read(),
                           inst_params.nonce()->content_size_get());
    return instantiate();
}

/*
 * Uninstantiate operation
 * instantiate with default personalization string and entropy
 */
bool
drbg_t::push(drbg_push_uninst_params_t& uninst_params)
{
    return uninstantiate();
}

/*
 * Re-instantiate operation, i.e.,
 * instantiate with default personalization string and entropy
 */
bool
drbg_t::push(drbg_push_reinst_params_t& reinst_params)
{
    string      psnl_str(dflt_psnl_str + to_string(inst));
    string      entropy(dflt_entropy + to_string(inst));

    psnl_str_set((const uint8_t *)psnl_str.c_str(),
                 psnl_str.size());
    entropy_input_inst_set((const uint8_t *)entropy.c_str(),
                           entropy.size(), nullptr, 0);
    return instantiate();
}

/*
 * Random bits generation
 */
bool
drbg_t::push(drbg_push_gen_params_t& gen_params)
{
    uint8_t     *buf;
    uint32_t    nbytes;
    bool        reseed = false;
    bool        success;

    buf = gen_params.output()->read();
    nbytes = gen_params.output()->line_size_get();

#ifdef HAS_NEW_DRBG_TESTVECTORS
    if (gen_params.add_input() &&
        gen_params.add_input()->content_size_get()) {
        OFFL_FUNC_ERR("drbg_t no HW support for AdditionalInput");
        return false;
    }
#endif
    if (gen_params.predict_resist_req()) {
        entropy_input_reseed_set(gen_params.entropy_pr()->read(),
                                 gen_params.entropy_pr()->content_size_get());
        reseed = !!gen_params.entropy_pr()->content_size_get();
    } else if (gen_params.entropy_reseed()) {
#ifdef HAS_NEW_DRBG_TESTVECTORS
        if (gen_params.add_input_reseed() &&
            gen_params.add_input_reseed()->content_size_get()) {
            OFFL_FUNC_ERR("drbg_t no HW support for AdditionalInputReseed");
            return false;
        }
#endif
        entropy_input_reseed_set(gen_params.entropy_reseed()->read(),
                                 gen_params.entropy_reseed()->content_size_get());
        reseed = !!gen_params.entropy_reseed()->content_size_get();
    }
    success = generate(buf, nbytes, gen_params.random_num_select(), reseed);
    if (success) {
        gen_params.output()->content_size_set(nbytes);
        gen_params.output()->write_thru();
    }
    return success;
}


/*
 * Check status for completion
 */
bool
drbg_t::completion_check(void)
{
    auto status_busy_check = [this] () -> int
    {
        if (status.busy_check()) {
            return 1;
        }
        return 0;
    };

#ifdef __x86_64__
    utils::Poller poll(poll_interval());
#else
    utils::Poller poll;
#endif
    if (poll(status_busy_check, false)) {
        OFFL_FUNC_ERR("drbg_t status is busy");
        return false;
    }

    return status.success_check();
}

/*
 * DRBG Instantiation
 */
bool
drbg_t::instantiate(void)
{
    rng_cfg &= ~(DRBG_RNG_NDRNG_MASK             |
                 DRBG_RNG_DRNG_MASK              |
                 DRBG_RNG_PSIZE_MASK             |
                 DRBG_RNG_TEST_DRNG_MASK         |
                 DRBG_RNG_START_MASK);
    rng_cfg |= DRBG_RNG_NDRNG_DIS                |
               DRBG_RNG_DRNG_INST                |
               DRBG_RNG_PSIZE(psnl_str_nbytes)   |
               DRBG_RNG_TEST_DRNG_CRYPTORAM      |
               DRBG_RNG_START_SET0;
    OFFL_FUNC_DEBUG("rng_cfg {:#x}", rng_cfg);

    status.init();
    CRYPTO_CTL_DRBG_WR(rng, rng_cfg);
    return completion_check();
}

/*
 * DRBG Unonstantiation
 */
bool
drbg_t::uninstantiate(void)
{
    rng_cfg &= ~(DRBG_RNG_NDRNG_MASK             |
                 DRBG_RNG_DRNG_MASK              |
                 DRBG_RNG_PSIZE_MASK             |
                 DRBG_RNG_TEST_DRNG_MASK         |
                 DRBG_RNG_START_MASK);
    rng_cfg |= DRBG_RNG_DRNG_UNINST              |
               DRBG_RNG_START_SET0;
    OFFL_FUNC_DEBUG("rng_cfg {:#x}", rng_cfg);

    status.init();
    CRYPTO_CTL_DRBG_WR(rng, rng_cfg);
    return completion_check();
}

/*
 * DRBG Generation
 */
bool
drbg_t::generate(uint8_t *buf,
                 uint32_t nbytes,
                 uint32_t random_num_select,
                 bool reseed)
{
    uint8_t     *p;
    uint32_t    total_nbytes;
    uint32_t    curr_nbytes;
    uint32_t    num_words;
    uint32_t    num_rands;
    bool        random_num0_select;
    bool        complete;

    random_num0_select = crypto_ctl_drbg_random_num0_select(random_num_select);
    num_words = CRYPTO_CTL_DRBG_CEIL_B2W(nbytes);

    rng_cfg &= ~(DRBG_RNG_NDRNG_MASK            |
                 DRBG_RNG_DRNG_MASK             |
                 DRBG_RNG_TEST_DRNG_MASK        |
                 DRBG_RNG_START_MASK);
    rng_cfg |= DRBG_RNG_NDRNG_DIS               |
               DRBG_RNG_DRNG_INT_STATE_S        |
               DRBG_RNG_TEST_DRNG_CRYPTORAM     |
               (random_num0_select ? DRBG_RNG_START_SET0 : DRBG_RNG_START_SET1);
    OFFL_FUNC_DEBUG("rng_cfg {:#x}", rng_cfg);

    /*
     * Each generation is random_bdepth bytes;
     * do multiple times if request size is greater than that.
     */
    p = buf;
    total_nbytes = nbytes;
    num_rands = CRYPTO_CTL_DRBG_CEIL_W2R(num_words, regs.random_wdepth0);
    while (num_rands--) {
        status.init();
        if (reseed) {
            CRYPTO_CTL_DRBG_WR(gct, DRBG_GCT_DRNG_PREDICT_EN);
        }
        CRYPTO_CTL_DRBG_WR(rng, rng_cfg);

        /*
         * DRBG HW is not reentrant so we make the operation
         * always run to completion.
         */
        complete = completion_check();
        if (reseed) {
            CRYPTO_CTL_DRBG_WR(gct, 0);
            reseed = false;
        }
        if (!complete) {
            break;
        }

        curr_nbytes = total_nbytes > random_bdepth ? 
                      random_bdepth : total_nbytes;
        if (random_num0_select) {
            rand_num0_get(p, curr_nbytes);
        } else {
            rand_num1_get(p, curr_nbytes);
        }
        p += curr_nbytes;
        total_nbytes -= curr_nbytes;
    }

    if (OFFL_IS_LOG_LEVEL_DEBUG()) {
        OFFL_FUNC_DEBUG("random number size {}", nbytes);
        utils::dump(buf, nbytes);
    }
    return true;
}

/*
 * Set personalization string
 */
void
drbg_t::psnl_str_set(const uint8_t *str,
                     uint32_t str_nbytes)
{
    uint32_t    psnl_str_bdepth = CRYPTO_CTL_DRBG_W2B(regs.psnl_wdepth);

    /*
     * FIPS testing allows an empty personalization string (if applicable)
     */
    memset((uint8_t *)psnl_str_cache, 0, psnl_str_bdepth);
    psnl_str_nbytes = min(str_nbytes, psnl_str_bdepth);
    memcpy((uint8_t *)psnl_str_cache, str, psnl_str_nbytes);

    if (OFFL_IS_LOG_LEVEL_DEBUG()) {
        OFFL_FUNC_DEBUG("size {}", psnl_str_nbytes);
        utils::dump((uint8_t *)str, psnl_str_nbytes);
    }
    for (uint32_t i = 0; i < regs.psnl_wdepth; i++) {
        CRYPTO_CTL_DRBG_WR_RAM(psnl_str_p, i, psnl_str_cache[i]);
    }
}

/*
 * Set entropy input for instantiation
 */
void
drbg_t::entropy_input_inst_set(const uint8_t *entropy,
                               uint32_t entropy_nbytes,
                               const uint8_t *nonce,
                               uint32_t nonce_nbytes)
{
    uint32_t    entropy_bdepth = CRYPTO_CTL_DRBG_W2B(regs.entropy_inst_wdepth);
    uint32_t    copy_nbytes;

    /*
     * Concatenate entropy (required) and nonce (optional)
     */
    if (entropy_nbytes) {
        memset((uint8_t *)entropy_inst_cache, 0, entropy_bdepth);
        copy_nbytes = min(entropy_nbytes, entropy_bdepth);
        entropy_input_nbytes = copy_nbytes;
        memcpy((uint8_t *)entropy_inst_cache, entropy, copy_nbytes);
        if (nonce) {
            copy_nbytes = min(entropy_bdepth - copy_nbytes, nonce_nbytes);
            memcpy((uint8_t *)entropy_inst_cache + entropy_input_nbytes,
                   nonce, copy_nbytes);
            entropy_input_nbytes += copy_nbytes;
        }

        if (OFFL_IS_LOG_LEVEL_DEBUG()) {
            OFFL_FUNC_DEBUG("size {}", entropy_input_nbytes);
            utils::dump((uint8_t *)entropy_inst_cache, entropy_input_nbytes);
        }
        for (uint32_t i = 0; i < regs.entropy_inst_wdepth; i++) {
            CRYPTO_CTL_DRBG_WR_RAM(entropy_inst, i, entropy_inst_cache[i]);
        }
        memset(entropy_inst_cache, 0, entropy_bdepth);
    }
}

/*
 * Set entropy input for reseed
 */
void
drbg_t::entropy_input_reseed_set(const uint8_t *add_input,
                                 uint32_t input_nbytes)
{
    uint32_t    reseed_bdepth = CRYPTO_CTL_DRBG_W2B(regs.entropy_reseed_wdepth);
    uint32_t    copy_nbytes;

    if (input_nbytes) {
        memset((uint8_t *)entropy_reseed_cache, 0, reseed_bdepth);
        copy_nbytes = min(input_nbytes, reseed_bdepth);
        memcpy((uint8_t *)entropy_reseed_cache, add_input, copy_nbytes);

        if (OFFL_IS_LOG_LEVEL_DEBUG()) {
            OFFL_FUNC_DEBUG("size {}", copy_nbytes);
            utils::dump((uint8_t *)entropy_reseed_cache, copy_nbytes);
        }
        for (uint32_t i = 0; i < regs.entropy_reseed_wdepth; i++) {
            CRYPTO_CTL_DRBG_WR_RAM(entropy_reseed, i, entropy_reseed_cache[i]);
        }
        memset(entropy_reseed_cache, 0, reseed_bdepth);
    }
}

/*
 * Fill argument buffer with generated random number from random_num0.
 */
void
drbg_t::rand_num0_get(uint8_t *buf,
                      uint32_t nbytes)
{
    uint32_t    num_words;

    num_words = CRYPTO_CTL_DRBG_CEIL_B2W(nbytes > random_bdepth ? 
                                         random_bdepth : nbytes);
    for (uint32_t i = 0; i < num_words; i++) {
        CRYPTO_CTL_DRBG_RD_RAM(random_num0, i, rand_num0_cache[i]);
    }
    memcpy(buf, rand_num0_cache, nbytes);
}

/*
 * Fill argument buffer with generated random number from random_num1.
 */
void
drbg_t::rand_num1_get(uint8_t *buf,
                      uint32_t nbytes)
{
    uint32_t    num_words;

    num_words = CRYPTO_CTL_DRBG_CEIL_B2W(nbytes > random_bdepth ? 
                                         random_bdepth : nbytes);
    for (uint32_t i = 0; i < num_words; i++) {
        CRYPTO_CTL_DRBG_RD_RAM(random_num1, i, rand_num1_cache[i]);
    }
    memcpy(buf, rand_num1_cache, nbytes);
}

/*
 * DRBG status
 */
status_t::status_t(const drbg_regs_t& regs) :
    regs(regs)
{
    init();
}

status_t::~status_t()
{
}

void
status_t::init(void)
{
    /*
     * Initialize status for later polling
     */
    gs_val = DRBG_GS_RNG_BUSY;
    isr_val = 0;
    drnge_val = 0;
}


bool
status_t::busy_check(void)
{
    CRYPTO_CTL_DRBG_RD(gs, gs_val);
    return !!(gs_val & DRBG_GS_RNG_BUSY);
}


bool
status_t::success_check(bool failure_expected)
{
    /*
     * clear interrupt and check for errors
     */
    CRYPTO_CTL_DRBG_RD(isr, isr_val);
    CRYPTO_CTL_DRBG_RD(drnge, drnge_val);

    if (drnge_val & DRBG_DRNGE_ERROR_MASK) {

        if (drnge_val & DRBG_DRNGE_ERROR_FUNC) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                     "functional error (catastrophic)");
        }
        if (drnge_val & DRBG_DRNGE_ERROR_HEALTH) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                     "error during instantiate/reseed/generate/uninstantiate");
        }
        return false;
    }
    return true;
}

/*
 * Access methods for PSE Openssl engine
 */
extern "C" {

static int
bytes(void *ctx,
      const PSE_RAND_BYTES_PARAM *param)
{
    drbg_t      *drbg = static_cast<drbg_t *>(ctx);
    bool        success;

    success = drbg->generate(param->buf, param->size);
    return success ? 1 : -1;
}

const PSE_OFFLOAD_RAND_METHOD pse_rand_offload_method =
{
    .bytes      = bytes,
    .mem_method = &pse_mem_method,
};

} // extern "C"

} // namespace crypto_drbg

