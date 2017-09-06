#include <hal_state_pd.hpp>
#include <capri_hbm.hpp>
#include <pd_api.hpp>
#include <capri_barco_crypto.hpp>
#include <p4plus_pd_api.h>

#define MAX_IPSEC_PAD_SIZE 256

namespace hal {
namespace pd {

uint64_t    key_mem_base = 0;
uint64_t    key_mem_size = 0;
char        key_mem[] = CAPRI_BARCO_KEY_MEM;

hal_ret_t pd_crypto_alloc_key(int32_t *key_idx)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     is = indexer::SUCCESS;
    uint64_t            key_addr = 0;

    is = g_hal_state_pd->crypto_pd_keys_idxr()->alloc((uint32_t*)key_idx);
    if (is != indexer::SUCCESS) {
        HAL_TRACE_ERR("SessKey: Failed to allocate key memory");
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


hal_ret_t pd_crypto_free_key(int32_t key_idx)
{
    hal_ret_t           ret = HAL_RET_OK;
    indexer::status     is = indexer::SUCCESS;

    if (!g_hal_state_pd->crypto_pd_keys_idxr()->is_alloced(key_idx)) {
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


hal_ret_t pd_crypto_write_key(int32_t key_idx, crypto_key_t *key)
{
    hal_ret_t           ret = HAL_RET_OK;

    if (ret == HAL_RET_OK) {
        ret = capri_barco_setup_key(key_idx, key->key_type, key->key, key->key_size);
    }

    return ret;
}

hal_ret_t pd_crypto_read_key(int32_t key_idx, crypto_key_t *key)
{
    hal_ret_t           ret = HAL_RET_OK;

    ret = capri_barco_read_key(key_idx, &key->key_type, key->key, &key->key_size);

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
        p4plus_hbm_write(ipsec_pad_base_addr, ipsec_pad_bytes, MAX_IPSEC_PAD_SIZE);
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

    ret = capri_barco_crypto_init();
    if (ret != HAL_RET_OK) {
        return ret;
    }
    ret = crypto_init_ipsec_pad_table();

    return ret;
}


}    // namespace pd
}    // namespace hal
