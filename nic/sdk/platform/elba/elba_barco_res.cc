// {C} Copyright 2020 Pensando Systems Inc. All rights reserved

#include "platform/elba/elba_barco_res.hpp"
#include "platform/utils/mpartition.hpp"
#include "asic/pd/pd.hpp"
#include "asic/cmn/asic_hbm.hpp"

namespace sdk {
namespace platform {
namespace elba {

slab *g_hal_elba_barco_pend_req_pd_slab = NULL;
thread_local dllist_ctxt_t g_pend_req_list;

elba_barco_resources_t elba_barco_resources[] = {
    /* 0 - CRYPTO_BARCO_RES_ASYM_DMA_DESCR */
    {
        "Crypto Asym DMA Descriptors",
        ASIC_HBM_REG_CRYPTO_ASYM_DMA_DESCR,
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
        ASIC_HBM_REG_CRYPTO_HBM_MEM,
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
        ASIC_HBM_REG_ASYM_KEY_DESCR,
        CRYPTO_ASYM_KEY_DESCR_COUNT_MAX,
        16,
        16,
        0,
        0,
        NULL
    },
    /* 3 - CRYPTO_BARCO_RES_SYM_MSG_DESCR */
    {
        "Crypto Sym Message Descriptors",
        ASIC_HBM_REG_CRYPTO_SYM_MSG_DESCR,
        CRYPTO_SYM_MSG_DESCR_COUNT_MAX,
        64,
        64,
        0,
        0,
        NULL
    },
};

#if 0
static inline indexer *
elba_barco_indexer_get (elba_barco_res_type_t res)
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
        case CRYPTO_BARCO_RES_SYM_MSG_DESCR:
            idxer = g_hal_state_pd->crypto_sym_msg_descr_idxr();
            break;
        default:
            SDK_TRACE_ERR("Invalid resource: %d", res);
            break;
    }
    assert(idxer != NULL);
    return idxer;
}
#endif

sdk_ret_t
elba_barco_obj_alloc (elba_barco_resources_t *elba_barco_res,
                      int32_t *res_id, uint64_t *res)
{
    uint32_t            idx = 0;
    indexer::status     is = indexer::SUCCESS;
    uint64_t            lres = 0;

    /* At least one of res_id or res needs to be valid */
    if (!res_id && !res) {
        return SDK_RET_INVALID_ARG;
    }

    if (res_id)
        *res_id = -1;

    if (res)
        *res = 0;

    is = elba_barco_res->idxer->alloc(&idx);
    if (is != indexer::SUCCESS) {
        SDK_TRACE_ERR("%s: Failed to allocate",
                      elba_barco_res->allocator_name);
        return SDK_RET_NO_RESOURCE;
    }

    if (res_id)
        *res_id = idx;

    lres = (elba_barco_res->hbm_region + (idx * elba_barco_res->obj_size));
    if (res)
        *res = lres;

    SDK_TRACE_DEBUG("%s: Allocated 0x%lx @ index: %u",
                    elba_barco_res->allocator_name, lres, idx);

    return SDK_RET_OK;

}

sdk_ret_t
elba_barco_obj_free_by_id (elba_barco_resources_t *elba_barco_res,
                           int32_t res_id)
{
    indexer::status     is = indexer::SUCCESS;

    if ((res_id < 0) || ((uint32_t)res_id >= elba_barco_res->obj_count)) {
        SDK_TRACE_ERR("%s: Invalid resource index: %d",
                      elba_barco_res->allocator_name, res_id);
        return SDK_RET_INVALID_ARG;
    }

    if (!elba_barco_res->idxer->is_index_allocated(res_id)) {
        SDK_TRACE_ERR("%s: Freeing unallocated descriptor: %d",
                      elba_barco_res->allocator_name, res_id);
        return SDK_RET_INVALID_ARG;
    }

    is = elba_barco_res->idxer->free(res_id);
    if (is != indexer::SUCCESS) {
        SDK_TRACE_ERR("%s: Failed to free memory @ %d",
                      elba_barco_res->allocator_name, res_id);
        return SDK_RET_INVALID_ARG;
    }

    SDK_TRACE_DEBUG("%s: Freed resource @ %d",
                    elba_barco_res->allocator_name, res_id);

    return SDK_RET_OK;
}

sdk_ret_t
elba_barco_obj_free (elba_barco_resources_t *elba_barco_res, uint64_t res)
{
    int32_t             res_id = 0;

    if ((res < elba_barco_res->hbm_region) ||
        (res > (elba_barco_res->hbm_region + elba_barco_res->hbm_region_size -
                        elba_barco_res->obj_size))) {
        SDK_TRACE_ERR("%s: Invalid descriptor address: 0x%lx",
                      elba_barco_res->allocator_name, res);
        SDK_TRACE_ERR("HBM Region: 0x%lx, Region Size: %u, Obj Size: %u",
                      elba_barco_res->hbm_region,
                      elba_barco_res->hbm_region_size,
                      elba_barco_res->obj_size);
        return SDK_RET_INVALID_ARG;

    }

    res_id = (res - elba_barco_res->hbm_region) / elba_barco_res->obj_size;

    return elba_barco_obj_free_by_id(elba_barco_res, res_id);
}

sdk_ret_t
elba_barco_res_region_get (elba_barco_res_type_t region_type,
                           uint64_t *region)
{
    if ((region_type >= CRYPTO_BARCO_RES_MIN) &&
            (region_type < CRYPTO_BARCO_RES_MAX)) {

        *region = elba_barco_resources[region_type].hbm_region;
        return SDK_RET_OK;
    }
    return SDK_RET_INVALID_ARG;
}

sdk_ret_t
elba_barco_res_region_size_get (elba_barco_res_type_t region_type,
                                uint16_t *region_size)
{
    if ((region_type >= CRYPTO_BARCO_RES_MIN) &&
            (region_type < CRYPTO_BARCO_RES_MAX)) {

        *region_size = elba_barco_resources[region_type].hbm_region_size;
        return SDK_RET_OK;
    }
    return SDK_RET_INVALID_ARG;
}

sdk_ret_t
elba_barco_res_obj_count_get (elba_barco_res_type_t region_type,
                              uint32_t *obj_count)
{
    if ((region_type >= CRYPTO_BARCO_RES_MIN) &&
            (region_type < CRYPTO_BARCO_RES_MAX)) {

        *obj_count = elba_barco_resources[region_type].obj_count;
        return SDK_RET_OK;
    }
    return SDK_RET_INVALID_ARG;
}

sdk_ret_t
elba_barco_res_allocator_init (void)
{
    uint16_t                idx;
    uint64_t                region = 0;
    uint32_t                region_size = 0;
    indexer                 *barco_indexers[CRYPTO_BARCO_RES_MAX];

    // slab
    if(!g_hal_elba_barco_pend_req_pd_slab) {
        g_hal_elba_barco_pend_req_pd_slab =
            slab::factory("CRYPTO PEND-REQ PD", sdk::lib::SDK_SLAB_ID_CRYPTO_PEND_REQ_PD,
                          sizeof(crypto_pend_req_t), 128,
                          true, true, true);
        SDK_ASSERT_RETURN(g_hal_elba_barco_pend_req_pd_slab != NULL,
                          SDK_RET_OOM);
    }
    dllist_init(&g_pend_req_list);

    barco_indexers[CRYPTO_BARCO_RES_ASYM_DMA_DESCR] =
        sdk::lib::indexer::factory(CRYPTO_ASYM_DMA_DESCR_COUNT_MAX);
    SDK_ASSERT_RETURN(barco_indexers[CRYPTO_BARCO_RES_ASYM_DMA_DESCR] != NULL,
                      SDK_RET_OOM);

     barco_indexers[CRYPTO_BARCO_RES_HBM_MEM_512B] =
         sdk::lib::indexer::factory(CRYPTO_HBM_MEM_COUNT_MAX);
     SDK_ASSERT_RETURN(barco_indexers[CRYPTO_BARCO_RES_HBM_MEM_512B] != NULL,
                       SDK_RET_OOM);

     barco_indexers[CRYPTO_BARCO_RES_ASYM_KEY_DESCR] =
         sdk::lib::indexer::factory(CRYPTO_ASYM_KEY_DESCR_COUNT_MAX);
     SDK_ASSERT_RETURN(barco_indexers[CRYPTO_BARCO_RES_ASYM_KEY_DESCR] != NULL,
                       SDK_RET_OOM);

     barco_indexers[CRYPTO_BARCO_RES_SYM_MSG_DESCR] =
         sdk::lib::indexer::factory(CRYPTO_SYM_MSG_DESCR_COUNT_MAX);
     SDK_ASSERT_RETURN(barco_indexers[CRYPTO_BARCO_RES_SYM_MSG_DESCR] != NULL,
                       SDK_RET_OOM);

    for (idx = CRYPTO_BARCO_RES_MIN; idx < CRYPTO_BARCO_RES_MAX; idx++) {
       region = sdk::asic::asic_get_mem_addr(elba_barco_resources[idx].hbm_region_name);
      //region = sdk::platform::elba::get_mem_addr(elba_barco_resources[idx].hbm_region_name);
      //  region = get_mem_addr(elba_barco_resources[idx].hbm_region_name);
        if (region == INVALID_MEM_ADDRESS) {
            SDK_TRACE_ERR("Failed to retrieve %s memory region",
                          elba_barco_resources[idx].allocator_name);
            return SDK_RET_ERR;
        }

        if (region & (elba_barco_resources[idx].obj_alignment - 1)) {
            SDK_TRACE_ERR("Failed to retrieve aligned memory region for %s",
                          elba_barco_resources[idx].allocator_name);
            return SDK_RET_ERR;
        }

        region_size =
            sdk::asic::asic_get_mem_size_kb(elba_barco_resources[idx].hbm_region_name) * 1024;
        if ((region_size/elba_barco_resources[idx].obj_size)
                < elba_barco_resources[idx].obj_count) {
            SDK_TRACE_ERR("Memory region not large enough for %s, got %d, required %d",
                          elba_barco_resources[idx].allocator_name, region_size,
                          elba_barco_resources[idx].obj_count *
                          elba_barco_resources[idx].obj_size);
            return SDK_RET_ERR;
        }
        elba_barco_resources[idx].hbm_region = region;
        elba_barco_resources[idx].hbm_region_size = region_size;
        elba_barco_resources[idx].idxer =
            barco_indexers[(elba_barco_res_type_t)idx];
        SDK_TRACE_DEBUG("Setting up %u %s @ 0x%lx",
                        elba_barco_resources[idx].obj_count,
                        elba_barco_resources[idx].allocator_name, region);
    }
    return SDK_RET_OK;
}

sdk_ret_t
elba_barco_res_alloc (elba_barco_res_type_t res_type, int32_t *res_id,
                      uint64_t *res)
{
    if ((res_type < CRYPTO_BARCO_RES_MIN) ||
            (res_type >= CRYPTO_BARCO_RES_MAX)) {
        return SDK_RET_INVALID_ARG;
    }

    return elba_barco_obj_alloc(&elba_barco_resources[res_type],
            res_id, res);
}

sdk_ret_t
elba_barco_res_free (elba_barco_res_type_t res_type, uint64_t res)
{
    if ((res_type < CRYPTO_BARCO_RES_MIN) ||
            (res_type >= CRYPTO_BARCO_RES_MAX)) {
        return SDK_RET_INVALID_ARG;
    }

    return elba_barco_obj_free(&elba_barco_resources[res_type], res);
}

sdk_ret_t
elba_barco_res_free_by_id (elba_barco_res_type_t res_type, int32_t res_id)
{
    if ((res_type < CRYPTO_BARCO_RES_MIN) ||
            (res_type >= CRYPTO_BARCO_RES_MAX)) {
        return SDK_RET_INVALID_ARG;
    }

    return elba_barco_obj_free_by_id(&elba_barco_resources[res_type], res_id);
}

sdk_ret_t
elba_barco_res_get_by_id (elba_barco_res_type_t region_type,
                          int32_t res_id, uint64_t *res)
{
    elba_barco_resources_t     *elba_barco_res;

    if ((region_type < CRYPTO_BARCO_RES_MIN) ||
            (region_type >= CRYPTO_BARCO_RES_MAX)) {
        return SDK_RET_INVALID_ARG;
    }
    elba_barco_res = &elba_barco_resources[region_type];

    if ((res_id < 0) || ((uint32_t)res_id >= elba_barco_res->obj_count)) {
        return SDK_RET_INVALID_ARG;
    }

    *res = (elba_barco_res->hbm_region + (res_id * elba_barco_res->obj_size));
    return SDK_RET_OK;
}

sdk_ret_t
elba_barco_add_pend_req_to_db (uint32_t hw_id, uint32_t sw_id)
{
    crypto_pend_req_t* req =
        (crypto_pend_req_t *)g_hal_elba_barco_pend_req_pd_slab->alloc();
    if(!req) {
        SDK_TRACE_ERR("Failed to allocate the req");
        return SDK_RET_OOM;
    }

    req->hw_id = hw_id;
    req->sw_id = sw_id;
    dllist_init(&req->list_ctxt);

    dllist_add_tail(&g_pend_req_list, &req->list_ctxt);
    return SDK_RET_OK;
}

sdk_ret_t
elba_barco_del_pend_req_from_db (crypto_pend_req_t *req)
{
    if(!req) {
        return SDK_RET_OK;
    }

    dllist_del(&req->list_ctxt);
    g_hal_elba_barco_pend_req_pd_slab->free(req);
    return SDK_RET_OK;
}

}    // namespace elba
}    // namespace platform
}    // namespace sdk
