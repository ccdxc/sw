#include "nic/hal/pd/iris/hal_state_pd.hpp"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"
#include "nic/hal/pd/capri/capri_barco_crypto.hpp"
#include "nic/hal/pd/capri/capri_barco_res.hpp"

#define MAX_IPSEC_PAD_SIZE 256

namespace hal {
namespace pd {

uint64_t    key_mem_base = 0;
uint64_t    key_mem_size = 0;
char        key_mem[] = CAPRI_BARCO_KEY_MEM;

// hal_ret_t pd_crypto_alloc_key(int32_t *key_idx)
hal_ret_t
pd_crypto_alloc_key(pd_crypto_alloc_key_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     is = indexer::SUCCESS;
    uint64_t            key_addr = 0;
    int32_t *key_idx = args->key_idx;

    is = g_hal_state_pd->crypto_pd_keys_idxr()->alloc((uint32_t *)key_idx);
    if (is != indexer::SUCCESS) {
        HAL_TRACE_ERR("SessKey: Failed to allocate key memory");
        *key_idx = -1;
        return HAL_RET_NO_RESOURCE;
    }
    /* Setup the key descriptor with the corresponding key memory
    *  Currently statically carved and associated 
    */
    key_addr = key_mem_base + (*key_idx * CRYPTO_KEY_SIZE_MAX);
    capri_barco_init_key(*key_idx, key_addr);
    HAL_TRACE_DEBUG("SessKey:{}: Allocated key memory @ index: {}", 
                    __FUNCTION__, *key_idx);
    return ret;
}


// hal_ret_t pd_crypto_free_key(int32_t key_idx)
hal_ret_t pd_crypto_free_key(pd_crypto_free_key_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     is = indexer::SUCCESS;
    int32_t key_idx = args->key_idx;

    if (!g_hal_state_pd->crypto_pd_keys_idxr()->is_index_allocated(key_idx)) {
        HAL_TRACE_ERR("SessKey: Freeing non-allocated key: {}", key_idx);
        return HAL_RET_INVALID_ARG;
    }

    is = g_hal_state_pd->crypto_pd_keys_idxr()->free(key_idx);
    if (is != indexer::SUCCESS) {
        HAL_TRACE_ERR("SessKey: Failed to free key memory: {}", key_idx);
        return HAL_RET_INVALID_ARG;
    }

    HAL_TRACE_DEBUG("SessKey:{}: Freed key memory @ index: {}", 
                    __FUNCTION__, key_idx);
    return ret;
}


// hal_ret_t pd_crypto_write_key(int32_t key_idx, crypto_key_t *key)
hal_ret_t pd_crypto_write_key(pd_crypto_write_key_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    int32_t key_idx = args->key_idx;
    crypto_key_t *key = args->key;

    ret = capri_barco_setup_key(key_idx, key->key_type, key->key, key->key_size);

    return ret;
}

// hal_ret_t pd_crypto_read_key(int32_t key_idx, crypto_key_t *key)
hal_ret_t pd_crypto_read_key(pd_crypto_read_key_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    int32_t key_idx = args->key_idx;
    crypto_key_t *key = args->key;

    ret = capri_barco_read_key(key_idx, &key->key_type, key->key, &key->key_size);

    return ret;
}

// hal_ret_t pd_crypto_asym_alloc_key(int32_t *key_idx)
hal_ret_t
pd_crypto_asym_alloc_key(pd_crypto_asym_alloc_key_args_t *args)

{
    uint64_t        key_desc;
    hal_ret_t       ret = HAL_RET_OK;
    int32_t *key_idx = args->key_idx;

    ret = capri_barco_res_alloc(CRYPTO_BARCO_RES_ASYM_KEY_DESCR, key_idx, &key_desc);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("SessKey: Failed to allocate key memory");
        *key_idx = -1;
        return HAL_RET_NO_RESOURCE;
    }
    return ret;
}

// hal_ret_t pd_crypto_asym_free_key(int32_t key_idx)
hal_ret_t pd_crypto_asym_free_key(pd_crypto_asym_free_key_args_t *args)
{
    hal_ret_t           ret = HAL_RET_OK;
    int32_t key_idx = args->key_idx;

    /* TODO: Also free up the DMA descriptor and corresponding memory regions 
     * if any referenced by the key descriptor 
     */

    ret = capri_barco_res_free_by_id(CRYPTO_BARCO_RES_ASYM_KEY_DESCR, key_idx);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("AsymKey: Failed to free key memory: {}", key_idx);
        return HAL_RET_INVALID_ARG;
    }

    return ret;
}

// hal_ret_t pd_crypto_asym_write_key(int32_t key_idx, crypto_asym_key_t *key)
hal_ret_t pd_crypto_asym_write_key(pd_crypto_asym_write_key_args_t *args)
{
    hal_ret_t                       ret = HAL_RET_OK;
    capri_barco_asym_key_desc_t     key_desc;
    uint64_t                        key_desc_addr = 0;
    int32_t key_idx = args->key_idx;
    crypto_asym_key_t *key = args->key;


    ret = capri_barco_res_get_by_id(CRYPTO_BARCO_RES_ASYM_KEY_DESCR, key_idx, &key_desc_addr);
    if (ret  != HAL_RET_OK) {
        HAL_TRACE_ERR("AsymKey Write: Failed to retrieve the address from key index: {}", key_idx);
        return ret;
    }

    key_desc.key_param_list = key->key_param_list;
    key_desc.command_reg = key->command_reg;

    if (capri_hbm_write_mem(key_desc_addr, (uint8_t*)&key_desc, sizeof(key_desc))) {
        HAL_TRACE_ERR("Failed to write Barco Asym key descriptor @ {:x}", (uint64_t) key_desc_addr); 
        return HAL_RET_INVALID_ARG;
    }
    HAL_TRACE_DEBUG("AsymKey Write: Setup key @ {:x}", key_desc_addr); 

    return ret;
}

// hal_ret_t pd_crypto_asym_read_key(int32_t key_idx, crypto_asym_key_t *key)
hal_ret_t pd_crypto_asym_read_key(pd_crypto_asym_read_key_args_t *args)
{
    hal_ret_t                       ret = HAL_RET_OK;
    capri_barco_asym_key_desc_t     key_desc;
    uint64_t                        key_desc_addr = 0;
    int32_t key_idx = args->key_idx;
    crypto_asym_key_t *key = args->key;

    ret = capri_barco_res_get_by_id(CRYPTO_BARCO_RES_ASYM_KEY_DESCR, key_idx, &key_desc_addr);
    if (ret  != HAL_RET_OK) {
        HAL_TRACE_ERR("AsymKey Read: Failed to retrieve the address from key index: {}", key_idx);
        return ret;
    }

    if (capri_hbm_read_mem(key_desc_addr, (uint8_t*)&key_desc, sizeof(key_desc))) {
        HAL_TRACE_ERR("Failed to read Barco Asym key descriptor from {:x}", (uint64_t) key_desc_addr); 
        return HAL_RET_INVALID_ARG;
    }

    key->key_param_list = key_desc.key_param_list;
    key->command_reg = key_desc.command_reg;

    return ret;
}

hal_ret_t
crypto_init_ipsec_pad_table(void)
{
    uint8_t ipsec_pad_bytes[MAX_IPSEC_PAD_SIZE];
    uint64_t ipsec_pad_base_addr = 0;

    HAL_TRACE_DEBUG("Initializing IPSEC Pad Bytes table"); 
    // Increasing number pattern as per RFC 1-255 
    for (int i = 0; i < MAX_IPSEC_PAD_SIZE; i++) {
        ipsec_pad_bytes[i] = i+1;
    }
     
    ipsec_pad_base_addr = get_start_offset(CAPRI_HBM_REG_IPSEC_PAD_TABLE);
    if (ipsec_pad_base_addr) {
        p4plus_hbm_write(ipsec_pad_base_addr, ipsec_pad_bytes, MAX_IPSEC_PAD_SIZE,
                P4PLUS_CACHE_ACTION_NONE);
    }
    return HAL_RET_OK;
}

hal_ret_t crypto_pd_init(void)
{
    hal_ret_t           ret = HAL_RET_OK;
    uint32_t            region_sz = 0;

    key_mem_base = get_start_offset(key_mem);
    region_sz = get_size_kb(key_mem) * 1024;
    key_mem_size = region_sz / CRYPTO_KEY_SIZE_MAX;
    assert(key_mem_size >= CRYPTO_KEY_COUNT_MAX);

    ret = crypto_init_ipsec_pad_table();
    if (ret != HAL_RET_OK) {
        return ret;
    }
    return ret;
}


}    // namespace pd
}    // namespace hal
