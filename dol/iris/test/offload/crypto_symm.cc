#include "crypto_symm.hpp"
#include "eng_if.hpp"
#include "hal_if.hpp"

/*
 * Symmetric Cryptography
 */
namespace crypto_symm {

acc_ring_t *mpp0_ring;

bool
init(const char *engine_path)
{
    sdk::platform::capri::barco_ring_meta_config_t   meta;

    if (!eng_if::init(engine_path)) {
        OFFL_FUNC_ERR("failed eng_if init");
        return false;
    }

    if (hal_if::get_ring_meta_config(types::BARCO_RING_MPP0, &meta) < 0) {
        OFFL_FUNC_ERR("failed symm ring meta config");
        return false;
    }

    OFFL_FUNC_INFO("mpp0_ring base {:#x} size {} pndx_addr {:#x} "
                   "shadow pndx addr {:#x}  opa_tag_addr {:#x} desc_size {} "
                   "pndx size {} opa_tag_size {}",
                   meta.ring_base, meta.ring_size, meta.producer_idx_addr,
                   meta.shadow_pndx_addr, meta.opaque_tag_addr, meta.desc_size,
                   meta.pndx_size, meta.opaque_tag_size);
    mpp0_ring = new acc_ring_t("mpp0_ring", meta.producer_idx_addr,
                                meta.shadow_pndx_addr, meta.ring_size,
                                meta.desc_size, meta.ring_base,
                                meta.pndx_size, meta.opaque_tag_addr,
                                meta.opaque_tag_size);
    return !!mpp0_ring;
}

/*
 * MSG descriptor pool
 */
msg_desc_pool_t::msg_desc_pool_t(msg_desc_pool_params_t& params) :

    params(params),
    hw_started(false),
    test_success(true)
{
    assert(params.num_descs());
    msg_desc_vec = new dp_mem_t(params.num_descs(),
                                sizeof(barco_msg_desc_t),
                                DP_MEM_ALIGN_SPEC,
                                params.msg_desc_mem_type(),
                                CRYPTO_SYMM_MSG_DESC_ALIGNMENT,
                                DP_MEM_ALLOC_NO_FILL);
}


msg_desc_pool_t::~msg_desc_pool_t()
{
    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.
    
    OFFL_FUNC_DEBUG("hw_started {} test_success {} destructor_free_buffers {}",
                    hw_started, test_success, 
                    params.base_params().destructor_free_buffers());
    if (params.base_params().destructor_free_buffers()) {
        if (test_success || !hw_started) {
            if (msg_desc_vec) delete msg_desc_vec;
        }
    }
}


uint64_t
msg_desc_pool_t::pre_push(msg_desc_pool_pre_push_params_t& pre_params)
{
    barco_msg_desc_t    *curr_msg_desc;
    dp_mem_t            *curr_msg_input;
    dp_mem_t            *next_msg_input;
    uint64_t            next_desc_pa = 0;
    uint32_t            curr_desc_idx = 0;
    uint32_t            curr_msg_idx = 0;

    auto msg_desc_get = [this, 
                         &curr_desc_idx,
                         &next_desc_pa] () -> barco_msg_desc_t *
    {
        barco_msg_desc_t    *msg_desc;
        uint32_t            next_desc_idx = curr_desc_idx + 1;

        next_desc_pa = 0;
        if (next_desc_idx < msg_desc_vec->num_lines_get()) {
            msg_desc_vec->line_set(next_desc_idx);
            next_desc_pa = msg_desc_vec->pa();
        }

        msg_desc = nullptr;
        if (curr_desc_idx < msg_desc_vec->num_lines_get()) {
            msg_desc_vec->line_set(curr_desc_idx);
            msg_desc_vec->clear();
            msg_desc = (barco_msg_desc_t *)msg_desc_vec->read();
            curr_desc_idx = next_desc_idx;
        }
        return msg_desc;
    };

    auto msg_input_get = [&pre_params, 
                          &curr_msg_idx] () -> dp_mem_t *
    {
        dp_mem_t    *msg_input;

        msg_input = nullptr;
        if (curr_msg_idx < pre_params.msg_input_vec().size()) {
            msg_input = pre_params.msg_input_vec().at(curr_msg_idx);
            curr_msg_idx++;
        }
        return msg_input;
    };

    curr_msg_desc = msg_desc_get();
    curr_msg_input = msg_input_get();

    while (curr_msg_desc && curr_msg_input) {
        next_msg_input = msg_input_get();
        if (!curr_msg_desc->A0_addr) {
            curr_msg_desc->A0_addr = curr_msg_input->pa();
            curr_msg_desc->L0_data_length = curr_msg_input->content_size_get();
            curr_msg_input = next_msg_input;
            continue;
        }
        if (!curr_msg_desc->A1_addr) {
            curr_msg_desc->A1_addr = curr_msg_input->pa();
            curr_msg_desc->L1_data_length = curr_msg_input->content_size_get();
            curr_msg_input = next_msg_input;
            continue;
        }
        curr_msg_desc->A2_addr = curr_msg_input->pa();
        curr_msg_desc->L2_data_length = curr_msg_input->content_size_get();

        if (next_msg_input) {
            curr_msg_desc->next_address = next_desc_pa;
            msg_desc_vec->write_thru();
            curr_msg_desc = msg_desc_get();
        }
        curr_msg_input = next_msg_input;
    }

    assert(!curr_msg_input);
    msg_desc_vec->write_thru();

    if (OFFL_IS_LOG_LEVEL_DEBUG()) {
        for (uint32_t i = 0; i < curr_desc_idx; i++) {
            msg_desc_vec->line_set(i);
            curr_msg_desc = (barco_msg_desc_t *)msg_desc_vec->read();
            OFFL_FUNC_DEBUG("msg {} addr {:#x} addr0 {:#x} len0 {} addr1 {:#x} "
                     "len1 {} addr2 {:#x} len2 {} next_addr {:#x}",
                     i, msg_desc_vec->pa(), curr_msg_desc->A0_addr,
                     curr_msg_desc->L0_data_length, curr_msg_desc->A1_addr,
                     curr_msg_desc->L1_data_length, curr_msg_desc->A2_addr,
                     curr_msg_desc->L2_data_length, curr_msg_desc->next_address);
        }
    }

    msg_desc_vec->line_set(0);
    return msg_desc_vec->pa();
}


bool
msg_desc_pool_t::push(void)
{
    hw_started = true;
    return true;
}


/*
 * Request descriptor
 */
bool
req_desc_t::pre_push(req_desc_pre_push_params_t& pre_params)
{
    req_desc = {0};
    req_desc.command = pre_params.cmd_pre_push().cmd_eval();

    req_desc.input_list_addr  = pre_params.src_msg_desc_addr();
    req_desc.output_list_addr = pre_params.dst_msg_desc_addr();
    req_desc.status_addr      = pre_params.status_addr();
    req_desc.iv_address       = pre_params.iv_addr();
    req_desc.auth_tag_addr    = pre_params.auth_tag_addr();
    if (key_idx_is_valid(pre_params.key_idx0())) {
        req_desc.key_descr_idx    = pre_params.key_idx0();
    }
    req_desc.opaque_tag_value = pre_params.opaque_tag_value();
    req_desc.opaque_tag_wr_en = pre_params.opaque_tag_en();
    req_desc.doorbell_addr    = pre_params.db_addr();
    req_desc.doorbell_data    = pre_params.db_data();
    req_desc.header_size      = pre_params.header_size();
    req_desc.sector_size      = pre_params.sector_size();
    req_desc.sector_num       = pre_params.sector_num();
    req_desc.application_tag  = pre_params.app_tag();
    if (key_idx_is_valid(pre_params.key_idx1())) {
        req_desc.second_key_descr_idx = pre_params.key_idx1();
    }

    OFFL_FUNC_DEBUG("req_desc cmd {:#x} input_list {:#x} output_list {:#x} "
                    "auth_tag_addr {:#x} status_addr {:#x} db_addr {:#x} db_data {:#x}",
                    req_desc.command, req_desc.input_list_addr,
                    req_desc.output_list_addr, req_desc.auth_tag_addr,
                    req_desc.status_addr, req_desc.doorbell_addr,
                    req_desc.doorbell_data);
    if (key_idx_is_valid(pre_params.key_idx0()) ||
        key_idx_is_valid(pre_params.key_idx1())) {
        OFFL_FUNC_DEBUG("key_idx0 {:#x} key_idx1 {:#x}",
                        req_desc.key_descr_idx, req_desc.second_key_descr_idx);
    }
    return true;
}


void *
req_desc_t::push(void)
{
    return &req_desc;
}


/*
 * crypto_symm command evaluator
 */
uint32_t
cmd_pre_push_params_t::cmd_eval(void)
{
    uint32_t    cmd = 0;

    switch (type()) {
    case CRYPTO_SYMM_TYPE_SHA:

        switch (sha_nbytes()) {
        case SHA_DIGEST_LENGTH:
            cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_HASH_SHA1;
            break;
        case SHA224_DIGEST_LENGTH:
            cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_HASH_SHA224;
            break;
        case SHA256_DIGEST_LENGTH:
            cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_HASH_SHA256;
            break;
        case SHA384_DIGEST_LENGTH:
            cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_HASH_SHA384;
            break;
        case SHA512_DIGEST_LENGTH:
            cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_HASH_SHA512;
            break;
        default:
             break;
        }
        break;

    case CRYPTO_SYMM_TYPE_SHA3:
        switch (sha_nbytes()) {
        case SHA224_DIGEST_LENGTH:
            cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_SHA3_224;
            break;
        case SHA256_DIGEST_LENGTH:
            cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_SHA3_256;
            break;
        case SHA384_DIGEST_LENGTH:
            cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_SHA3_384;
            break;
        case SHA512_DIGEST_LENGTH:
            cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_SHA3_512;
            break;
        default:
             break;
        }
        break;

    case CRYPTO_SYMM_TYPE_AES_CBC:
        cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_AES_CBC;
        break;

    case CRYPTO_SYMM_TYPE_AES_CCM:
        cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_AES_CCM;
        break;

    case CRYPTO_SYMM_TYPE_AES_GCM:
        cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_AES_GCM;
        break;

    case CRYPTO_SYMM_TYPE_AES_XTS:
        cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_AES_XTS;
        break;

    case CRYPTO_SYMM_TYPE_ECB:
        cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_DES_ECB;
        break;

    case CRYPTO_SYMM_TYPE_CBC:
        cmd |= CAPRI_BARCO_SYM_COMMAND_ALGO_DES_CBC;
        break;

    default:
        break;
    }

    switch (op()) {

    case CRYPTO_SYMM_OP_GENERATE:
    case CRYPTO_SYMM_OP_ENCRYPT:
        cmd |= 0 << CAPRI_BARCO_SYM_COMMAND_OPER_SHIFT;
        break;

    case CRYPTO_SYMM_OP_VERIFY:
    case CRYPTO_SYMM_OP_DECRYPT:
        cmd |= 1 << CAPRI_BARCO_SYM_COMMAND_OPER_SHIFT;
        break;

    default:
        break;
    }

    return cmd;
}

/*
 * Symm status
 */
status_t::status_t(dp_mem_type_t mem_type)
{
    symm_status = new dp_mem_t(1, sizeof(barco_symm_status_t),
                               DP_MEM_ALIGN_SPEC, mem_type,
                               CRYPTO_SYMM_STATUS_ALIGNMENT,
                               DP_MEM_ALLOC_NO_FILL);
    init();
}

status_t::~status_t()
{
    if (symm_status) {
        delete symm_status;
    }
}

void
status_t::init(void)
{
    /*
     * Initialize status for later polling
     */
    symm_status->clear_thru();
}


bool
status_t::busy_check(void)
{
    /*
     * Symmetric ring does not have busy status
     */
    return false;
}


bool
status_t::success_check(bool failure_expected)
{
    barco_symm_status_t *status =
               (barco_symm_status_t *)symm_status->read_thru();

    if (status->full_word) {

        if (status->full_word & CRYPTO_SYMM_T10_WRONG_PROT_INFO) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                                  "T10 protection information is wrong");
        }
        if (status->full_word & CRYPTO_SYMM_LEN_NOT_MULTIPLE) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                                  "length is not multiple of sector size");
        }
        return false;
    }
    return true;
}

/*
 * Symm doorbell
 */
doorbell_t::doorbell_t(dp_mem_type_t mem_type,
                       uint64_t data) :
    data_(data)
{
    symm_doorbell = new dp_mem_t(1, sizeof(barco_symm_doorbell_t),
                                 DP_MEM_ALIGN_SPEC, mem_type,
                                 CRYPTO_SYMM_DOORBELL_ALIGNMENT,
                                 DP_MEM_ALLOC_NO_FILL);
    init();
}

doorbell_t::~doorbell_t()
{
    if (symm_doorbell) {
        delete symm_doorbell;
    }
}

void
doorbell_t::init(void)
{
    barco_symm_doorbell_t *doorbell;

    /*
     * Initialize doorbell for later polling
     */
    symm_doorbell->clear();
    doorbell = (barco_symm_doorbell_t *)symm_doorbell->read();
    doorbell->data = ~data_;
    symm_doorbell->write_thru();
}


bool
doorbell_t::busy_check(void)
{
    barco_symm_doorbell_t *doorbell =
               (barco_symm_doorbell_t *)symm_doorbell->read_thru();
    return doorbell->data != data_;
}


bool
doorbell_t::success_check(bool failure_expected)
{
    return !busy_check();
}

} // namespace crypto_symm

