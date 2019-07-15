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

    dma_desc->stop = !dma_desc_idx_is_valid(pre_params.next_idx());
    dma_desc->realign = dma_desc->stop; // always set on last descriptor

    if (!dma_desc->stop) {
        assert(dma_desc_vec->num_lines_get() > pre_params.next_idx());
        dma_desc_vec->line_set(pre_params.next_idx());
        dma_desc->next = dma_desc_vec->pa() >> 2;
        dma_desc_vec->line_set(pre_params.desc_idx());
    }

    OFFL_FUNC_DEBUG("dma_desc {} addr {:#x} data_addr {:#x} next_desc {:#x}",
                    pre_params.desc_idx(), dma_desc_vec->pa(), 
                    dma_desc->address, dma_desc->next << 2);
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
                    "status_addr {:#x} key_idx {}",
                    req_desc.input_list_addr, req_desc.output_list_addr,
                    req_desc.status_addr, req_desc.key_descr_idx);
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
    if (pre_params.cmd_rsa_sig_gen()) {
        key_desc.command_reg |= CAPRI_BARCO_ASYM_CMD_RSA_SIG_GEN;
    } else if (pre_params.cmd_rsa_sig_verify()) {
        key_desc.command_reg |= CAPRI_BARCO_ASYM_CMD_RSA_SIG_VERIFY;
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

} // namespace crypto_asym

