#include "crypto_asym.hpp"
#include "dole_if.hpp"
#include "hal_if.hpp"

namespace crypto_asym {

acc_ring_t *asym_ring;

bool
init(const char *engine_path)
{
    hal::barco_ring_meta_config_t   meta;

    if (!dole_if::init(engine_path)) {
        OFFL_FUNC_ERR("failed dole_if init");
        return false;
    }

    if (hal_if::get_ring_meta_config(types::BARCO_RING_ASYM, &meta) < 0) {
        OFFL_FUNC_ERR("failed asym ring meta config");
        return false;
    }

    OFFL_FUNC_INFO("asym ring base {:#x} size {} pndx_addr {:#x} "
                   "shadow pndx addr {:#x}  opa_tag_addr {:#x} desc_size {} "
                   "pndx size {} opa_tag_size {}",
                   meta.ring_base, meta.ring_size, meta.producer_idx_addr,
                   meta.shadow_pndx_addr, meta.opaque_tag_addr, meta.desc_size,
                   meta.pndx_size, meta.opaque_tag_size);
    asym_ring = new acc_ring_t("asym_ring", meta.producer_idx_addr,
                               meta.shadow_pndx_addr, meta.ring_size,
                               meta.desc_size, meta.ring_base,
                               meta.pndx_size, meta.opaque_tag_addr,
                               meta.opaque_tag_size);
    return !!asym_ring;
}

/*
 * DMA descriptor pool
 */
dma_desc_pool_t::dma_desc_pool_t(dma_desc_pool_params_t& params) :

    params(params),
    hw_started(false),
    test_success(true)
{
    assert(params.num_descs());
    dma_desc_vec = new dp_mem_t(params.num_descs(),
                                CRYPTO_ASYM_DMA_DESC_ALIGNMENT,
                                DP_MEM_ALIGN_SPEC,
                                params.dma_desc_mem_type(),
                                CRYPTO_ASYM_DMA_DESC_ALIGNMENT,
                                DP_MEM_ALLOC_NO_FILL);
}


dma_desc_pool_t::~dma_desc_pool_t()
{
    // Only free buffers on successful completion; otherwise,
    // HW/P4+ might still be trying to access them.
    
    OFFL_FUNC_DEBUG("hw_started {} test_success {} destructor_free_buffers {}",
                    hw_started, test_success, 
                    params.base_params().destructor_free_buffers());
    if (params.base_params().destructor_free_buffers()) {
        if (test_success || !hw_started) {
            if (dma_desc_vec) delete dma_desc_vec;
        }
    }
}


uint64_t
dma_desc_pool_t::pre_push(dma_desc_pool_pre_push_params_t& pre_params)
{
    barco_dma_desc_t    *dma_desc;

    assert(dma_desc_idx_is_valid(pre_params.desc_idx()) &&
           pre_params.data());
    assert(dma_desc_vec->num_lines_get() > pre_params.desc_idx());

    dma_desc_vec->line_set(pre_params.desc_idx());
    dma_desc_vec->clear();
    dma_desc = (barco_dma_desc_t *)dma_desc_vec->read();
    dma_desc->address = pre_params.data()->pa();
    dma_desc->rsvd0 = true;
    dma_desc->length = pre_params.data()->content_size_get();

    /*
     * double_size means the caller data consist of 2 contiguous
     * chunks in memory.
     */
    if (pre_params.double_size()) {
        dma_desc->length *= 2;
    }

    dma_desc->stop = !dma_desc_idx_is_valid(pre_params.next_idx());
    dma_desc->realign = dma_desc->stop; // always set on last descriptor

    if (!dma_desc->stop) {
        assert(dma_desc_vec->num_lines_get() > pre_params.next_idx());
        dma_desc_vec->line_set(pre_params.next_idx());
        dma_desc->next = dma_desc_vec->pa() >> 2;
        dma_desc_vec->line_set(pre_params.desc_idx());
    }

    OFFL_FUNC_DEBUG("dma_desc {} addr {:#x} data_addr {:#x} next_desc {:#x} length {}",
                    pre_params.desc_idx(), dma_desc_vec->pa(), 
                    dma_desc->address, dma_desc->next << 2, dma_desc->length);
    dma_desc_vec->write_thru();
    return dma_desc_vec->pa();
}


bool
dma_desc_pool_t::push(void)
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
    req_desc.input_list_addr  = pre_params.input_list_addr();
    req_desc.output_list_addr = pre_params.output_list_addr();
    req_desc.status_addr      = pre_params.status_addr();
    req_desc.key_descr_idx    = pre_params.key_idx();
    req_desc.opaque_tag_value = pre_params.opaque_tag_value();
    req_desc.opage_tag_wr_en  = pre_params.opaque_tag_en();

    OFFL_FUNC_DEBUG("req_desc input_list {:#x} output_list {:#x} "
                    "status_addr {:#x} key_idx {} opa_tag_value {}",
                    req_desc.input_list_addr, req_desc.output_list_addr,
                    req_desc.status_addr, req_desc.key_descr_idx,
                    req_desc.opaque_tag_value);
    return true;
}


void *
req_desc_t::push(void)
{
    return &req_desc;
}

/*
 * Key descriptor
 */
bool
key_desc_t::pre_push(key_desc_pre_push_params_t& pre_params)
{
    key_desc = {0};
    key_desc.key_param_list = pre_params.key_param_list();
    if (pre_params.swap_bytes()) {
        key_desc.command_reg |= CAPRI_BARCO_ASYM_CMD_SWAP_BYTES;
    }

    /*
     * Only one of the following is expected to be true
     */
    if (pre_params.cmd_rsa_sign()) {
        key_desc.command_reg |= CAPRI_BARCO_ASYM_CMD_RSA_SIG_GEN;
    } else if (pre_params.cmd_rsa_verify()) {
        key_desc.command_reg |= CAPRI_BARCO_ASYM_CMD_RSA_SIG_VERIFY;
    } else if (pre_params.cmd_rsa_encrypt()) {
        key_desc.command_reg |= CAPRI_BARCO_ASYM_CMD_RSA_ENCRYPT;
    } else if (pre_params.cmd_rsa_decrypt()) {
        key_desc.command_reg |= CAPRI_BARCO_ASYM_CMD_RSA_DECRYPT;
    } else {
        OFFL_FUNC_ERR("failed to set command_reg");
        return false;
    }

    key_desc.command_reg |= CAPRI_BARCO_ASYM_CMD_SIZE_OF_OP(pre_params.cmd_size());
    OFFL_FUNC_DEBUG("key_desc key_param_list {:#x} command {:#x}",
                    key_desc.key_param_list, key_desc.command_reg);
    return true;
}


void *
key_desc_t::push(void)
{
    return &key_desc;
}


/*
 * Asym status
 */
status_t::status_t(dp_mem_type_t mem_type)
{
    asym_status = new dp_mem_t(1, sizeof(barco_asym_status_t),
                               DP_MEM_ALIGN_NONE, mem_type);
    init();
}

status_t::~status_t()
{
    if (asym_status) {
        delete asym_status;
    }
}

void
status_t::init(void)
{
    barco_asym_status_t *status;
    uint32_t            all_ones = ~0;

    /*
     * Initialize status for later polling
     */
    asym_status->clear();
    status = (barco_asym_status_t *)asym_status->read();
    status->pk_busy = true;
    status->err_flags = all_ones;
    asym_status->write_thru();
}


bool
status_t::busy_check(void)
{
    barco_asym_status_t *status =
               (barco_asym_status_t *)asym_status->read_thru();
    return status->pk_busy;
}


bool
status_t::success_check(bool failure_expected)
{
    barco_asym_status_t *status =
               (barco_asym_status_t *)asym_status->read_thru();

    if (status->err_flags) {

        if (status->full_word & CRYPTO_ASYM_STATUS_PPX_NOT_ON_CURVE) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                                  "point Px not on defined EC");
        }
        if (status->full_word & CRYPTO_ASYM_STATUS_PPX_AT_INFINITE) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                                  "point Px is at infinity");
        }
        if (status->full_word & CRYPTO_ASYM_STATUS_COUPLE_NOT_VALID) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                     "couple x, y is not valid (not smaller than prime)");
        }
        if (status->full_word & CRYPTO_ASYM_STATUS_PARAM_N_NOT_VALID) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                                  "parameter n is not valid");
        }
        if (status->full_word & CRYPTO_ASYM_STATUS_NOT_IMPLEMENTED) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                                  "commnand is not valid");
        }
        if (status->full_word & CRYPTO_ASYM_STATUS_SIG_NOT_VALID) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                                  "signature is not valid");
        }
        if (status->full_word & CRYPTO_ASYM_STATUS_PARAM_AB_NOT_VALID) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                                  "parameters A and B are not valid");
        }
        if (status->full_word & CRYPTO_ASYM_STATUS_NOT_INVERTIBLE) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                                  "modular inversion operand not invertible");
        }
        if (status->full_word & CRYPTO_ASYM_STATUS_COMPOSITE) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                                  "random number under test is composite");
        }
        if (status->full_word & CRYPTO_ASYM_STATUS_NOT_QUAD_RESIDUE) {
            OFFL_LOG_ERR_OR_DEBUG(failure_expected,
                     "modular square root operand not a quadratic residue");
        }
        return false;
    }
    return true;
}

} // namespace crypto_asym

