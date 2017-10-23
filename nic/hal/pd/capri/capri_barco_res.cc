#include "nic/hal/hal.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/capri/capri_barco_res.hpp"


namespace hal {

namespace pd {


capri_barco_resources_t capri_barco_resources[] = {
    /* 0 - CRYPTO_BARCO_RES_ASYM_DMA_DESCR */
    {
        "Crypto Asym DMA Descriptors",
        CAPRI_HBM_REG_CRYPTO_ASYM_DMA_DESCR,
        CRYPTO_ASYM_DMA_DESCR_COUNT_MAX,
        24,
        4,
        0,
        0,
        NULL
    },
    /* 1 - CRYPTO_BARCO_RES_HBM_MEM_512B */
    {
        "Crypto 512B HBM Mem Blocks",
        CAPRI_HBM_REG_CRYPTO_HBM_MEM,
        CRYPTO_HBM_MEM_COUNT_MAX,
        512,
        4,
        0,
        0,
        NULL
    },
    /* 2 - CRYPTO_BARCO_RES_ASYM_KEY_DESCR */
    {
        "Crypto Asym Key Descriptors",
        CAPRI_HBM_REG_ASYM_KEY_DESCR,
        CRYPTO_ASYM_KEY_DESCR_COUNT_MAX,
        16,
        16,
        0,
        0,
        NULL
    },
};

static inline indexer * capri_barco_indexer_get(capri_barco_res_type_t res)
{
    indexer     *idxer = NULL;

    switch (res) {
        case CRYPTO_BARCO_RES_ASYM_DMA_DESCR:
            idxer = g_hal_state_pd->crypto_asym_dma_descr_idxr();
            break;
        case CRYPTO_BARCO_RES_HBM_MEM_512B:
            idxer = g_hal_state_pd->hbm_mem_idxr();
            break;
        case CRYPTO_BARCO_RES_ASYM_KEY_DESCR:
            idxer = g_hal_state_pd->crypto_asym_key_descr_idxr();
            break;
        default:
            HAL_TRACE_ERR("Invalid resource: {}", res);
            break;
    }
    assert(idxer != NULL);
    return idxer;
}

hal_ret_t capri_barco_obj_alloc(capri_barco_resources_t *capri_barco_res,
        int32_t *res_id, uint64_t *res)
{
    uint32_t            idx = 0;
    indexer::status     is = indexer::SUCCESS;

    if (res_id)
        *res_id = -1;

    is = capri_barco_res->idxer->alloc((uint32_t*)&idx);
    if (is != indexer::SUCCESS) {
        HAL_TRACE_ERR("{}: Failed to allocate", capri_barco_res->allocator_name);
        return HAL_RET_NO_RESOURCE;
    }

    if (res_id)
        *res_id = idx;
    
    *res = (capri_barco_res->hbm_region + (idx * capri_barco_res->obj_size));

    HAL_TRACE_DEBUG("{}: Allocated {} @ index:{}",
            capri_barco_res->allocator_name, *res, idx);
    
    return HAL_RET_OK;

}

hal_ret_t capri_barco_obj_free_by_id(capri_barco_resources_t *capri_barco_res,
        int32_t res_id)
{
    indexer::status     is = indexer::SUCCESS;

    if ((res_id < 0) || ((uint32_t)res_id >= capri_barco_res->obj_count)) {
        HAL_TRACE_ERR("{}: Invalid resource index: {}",
                capri_barco_res->allocator_name, res_id);
        return HAL_RET_INVALID_ARG;
    }

    if (!capri_barco_res->idxer->is_alloced(res_id)) {
        HAL_TRACE_ERR("{}: Freeing unallocated descriptor: {}", capri_barco_res->allocator_name, res_id);
        return HAL_RET_INVALID_ARG;
    }

    is = capri_barco_res->idxer->free(res_id);
    if (is != indexer::SUCCESS) {
        HAL_TRACE_ERR("{}: Failed to free memory @ {}",
                capri_barco_res->allocator_name, res_id);
        return HAL_RET_INVALID_ARG;
    }
    
    HAL_TRACE_DEBUG("{}: Freed resource @ {}", capri_barco_res->allocator_name, res_id);

    return HAL_RET_OK;
}

hal_ret_t capri_barco_obj_free(capri_barco_resources_t *capri_barco_res, uint64_t res)
{
    int32_t             res_id = 0;

    if ((res < capri_barco_res->hbm_region) ||
        (res > (capri_barco_res->hbm_region + capri_barco_res->hbm_region_size -
                        capri_barco_res->obj_size))) {
        HAL_TRACE_ERR("{}: Invalid descriptor address: {}", capri_barco_res->allocator_name, res);
        return HAL_RET_INVALID_ARG;
        
    }

    res_id = (res - capri_barco_res->hbm_region) / capri_barco_res->obj_size;

    return capri_barco_obj_free_by_id(capri_barco_res, res_id);
}


hal_ret_t capri_barco_res_region_get(capri_barco_res_type_t region_type,
        uint64_t *region)
{
    if ((region_type >= CRYPTO_BARCO_RES_MIN) &&
            (region_type < CRYPTO_BARCO_RES_MAX)) {

        *region = capri_barco_resources[region_type].hbm_region;
        return HAL_RET_OK;
    }
    return HAL_RET_INVALID_ARG;
}

hal_ret_t capri_barco_res_region_size_get(capri_barco_res_type_t region_type,
        uint16_t *region_size)
{
    if ((region_type >= CRYPTO_BARCO_RES_MIN) &&
            (region_type < CRYPTO_BARCO_RES_MAX)) {

        *region_size = capri_barco_resources[region_type].hbm_region_size;
        return HAL_RET_OK;
    }
    return HAL_RET_INVALID_ARG;
}

hal_ret_t capri_barco_res_obj_count_get(capri_barco_res_type_t region_type,
        uint32_t *obj_count)
{
    if ((region_type >= CRYPTO_BARCO_RES_MIN) &&
            (region_type < CRYPTO_BARCO_RES_MAX)) {

        *obj_count = capri_barco_resources[region_type].obj_count;
        return HAL_RET_OK;
    }
    return HAL_RET_INVALID_ARG;
}

hal_ret_t capri_barco_res_allocator_init(void)
{
    uint16_t                idx;
    uint64_t                region = 0; 
    uint32_t                region_size = 0;


    for (idx = CRYPTO_BARCO_RES_MIN; idx < CRYPTO_BARCO_RES_MAX; idx++) {

        region = get_start_offset(capri_barco_resources[idx].hbm_region_name);
        if (!region) {
            HAL_TRACE_ERR("Failed to retrieve {} memory region",
                    capri_barco_resources[idx].allocator_name);
            return HAL_RET_ERR;
        }

        if (region & (capri_barco_resources[idx].obj_alignment - 1)) {
            HAL_TRACE_ERR("Failed to retrieve aligned memory region for {}",
                    capri_barco_resources[idx].allocator_name);
            return HAL_RET_ERR;
        }

        region_size = get_size_kb(capri_barco_resources[idx].hbm_region_name) * 1024;
        if ((region_size/capri_barco_resources[idx].obj_size)
                < capri_barco_resources[idx].obj_count) {
            HAL_TRACE_ERR("Memory region not large enough for {}, got {}, required {}",
                    capri_barco_resources[idx].allocator_name, region_size,
                    capri_barco_resources[idx].obj_count * capri_barco_resources[idx].obj_size);
            return HAL_RET_ERR;
        }
        capri_barco_resources[idx].hbm_region = region;
        capri_barco_resources[idx].hbm_region_size = region_size;
        capri_barco_resources[idx].idxer =
            capri_barco_indexer_get((capri_barco_res_type_t)idx);
        HAL_TRACE_DEBUG("Setting up {} {} @ {:x}",
                capri_barco_resources[idx].obj_count,
                capri_barco_resources[idx].allocator_name,
                region);
    }
    return HAL_RET_OK;
}


hal_ret_t capri_barco_res_alloc(capri_barco_res_type_t res_type, int32_t *res_id,
        uint64_t *res)
{
    if ((res_type < CRYPTO_BARCO_RES_MIN) ||
            (res_type >= CRYPTO_BARCO_RES_MAX)) {
        return HAL_RET_INVALID_ARG;
    }

    return capri_barco_obj_alloc(&capri_barco_resources[res_type], 
            res_id, res);
}

hal_ret_t capri_barco_res_free(capri_barco_res_type_t res_type, uint64_t res)
{
    if ((res_type < CRYPTO_BARCO_RES_MIN) ||
            (res_type >= CRYPTO_BARCO_RES_MAX)) {
        return HAL_RET_INVALID_ARG;
    }

    return capri_barco_obj_free(&capri_barco_resources[res_type], res);
}

hal_ret_t capri_barco_res_free_by_id(capri_barco_res_type_t res_type, int32_t res_id)
{
    if ((res_type < CRYPTO_BARCO_RES_MIN) ||
            (res_type >= CRYPTO_BARCO_RES_MAX)) {
        return HAL_RET_INVALID_ARG;
    }

    return capri_barco_obj_free_by_id(&capri_barco_resources[res_type], res_id);
}

hal_ret_t capri_barco_res_get_by_id(capri_barco_res_type_t region_type,
        int32_t res_id, uint64_t *res)
{
    capri_barco_resources_t     *capri_barco_res;

    if ((region_type < CRYPTO_BARCO_RES_MIN) ||
            (region_type >= CRYPTO_BARCO_RES_MAX)) {
        return HAL_RET_INVALID_ARG;
    }
    capri_barco_res = &capri_barco_resources[region_type];

    if ((res_id < 0) || ((uint32_t)res_id >= capri_barco_res->obj_count)) {
        return HAL_RET_INVALID_ARG;
    }

    *res = (capri_barco_res->hbm_region + (res_id * capri_barco_res->obj_size));
    return HAL_RET_OK;
}




} // namespace pd

} // namespace hal
