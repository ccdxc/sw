#include <hal_lock.hpp>
#include <hal_state_pd.hpp>
#include <buf_pool_pd.hpp>
#include <pd_api.hpp>
#include <qos_api.hpp>
#include <capri_tm_rw.hpp>

namespace hal {
namespace pd {

hal_ret_t
pd_buf_pool_create (pd_buf_pool_args_t *args) 
{
    hal_ret_t     ret;
    pd_buf_pool_t *pd_buf_pool;

    HAL_TRACE_DEBUG("PD-BUF-POOL::{}: Creating pd state for buf-pool: {}", 
            __func__, buf_pool_get_buf_pool_id(args->buf_pool));

    // Create buf_pool PD
    pd_buf_pool = buf_pool_pd_alloc_init();
    if (pd_buf_pool == NULL) {
        ret = HAL_RET_OOM;
        goto end;
    }

    // Link PI & PD
    link_pi_pd(pd_buf_pool, args->buf_pool);

    // Allocate Resources
    ret = buf_pool_pd_alloc_res(pd_buf_pool);
    if (ret != HAL_RET_OK) {
        // No Resources, dont allocate PD
        HAL_TRACE_ERR("PD-BUF-POOL::{}: Unable to alloc. resources for buf-pool: {}",
                __func__, buf_pool_get_buf_pool_id(args->buf_pool));
        goto end;
    }

    // Program HW
    ret = buf_pool_pd_program_hw(pd_buf_pool);

end:
    if (ret != HAL_RET_OK) {
        unlink_pi_pd(pd_buf_pool, args->buf_pool);
        buf_pool_pd_free(pd_buf_pool);
    }
    return ret;
}

// ----------------------------------------------------------------------------
// Allocate and Initialize Buf-Pool PD Instance
// ----------------------------------------------------------------------------
inline pd_buf_pool_t *
buf_pool_pd_alloc_init (void)
{
    return buf_pool_pd_init(buf_pool_pd_alloc());
}

// ----------------------------------------------------------------------------
// Allocate Buf-Pool Instance
// ----------------------------------------------------------------------------
inline pd_buf_pool_t *
buf_pool_pd_alloc (void)
{
    pd_buf_pool_t    *buf_pool;

    buf_pool = (pd_buf_pool_t *)g_hal_state_pd->buf_pool_pd_slab()->alloc();
    if (buf_pool == NULL) {
        return NULL;
    }
    return buf_pool;
}

// ----------------------------------------------------------------------------
// Initialize Buf-Pool PD instance
// ----------------------------------------------------------------------------
inline pd_buf_pool_t *
buf_pool_pd_init (pd_buf_pool_t *buf_pool)
{
    // Nothing to do currently
    if (!buf_pool) {
        return NULL;
    }

    // Set here if you want to initialize any fields

    return buf_pool;
}

// ----------------------------------------------------------------------------
// Allocate resources for PD Buf-Pool
// ----------------------------------------------------------------------------
hal_ret_t 
buf_pool_pd_alloc_res(pd_buf_pool_t *pd_buf_pool)
{
    hal_ret_t            ret = HAL_RET_OK;
    indexer::status      rs = indexer::SUCCESS;
    uint32_t             port_num;

    port_num = buf_pool_get_port_num((buf_pool_t *)(pd_buf_pool->pi_buf_pool));

    // Allocate buf_pool hwid
    rs = g_hal_state_pd->buf_pool_hwid_idxr(port_num)->alloc(
                                    (uint32_t *)&pd_buf_pool->hw_buf_pool_id);
    if (rs != indexer::SUCCESS) {
        return HAL_RET_NO_RESOURCE;
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Program HW
// ----------------------------------------------------------------------------
hal_ret_t
buf_pool_pd_program_hw(pd_buf_pool_t *pd_buf_pool)
{
    hal_ret_t      ret       = HAL_RET_OK;
    buf_pool_t     *buf_pool = (buf_pool_t *)pd_buf_pool->pi_buf_pool;
    uint32_t       tm_port   = buf_pool_get_port_num(buf_pool);
    uint32_t       pg        = pd_buf_pool->hw_buf_pool_id;
    tm_pg_params_t pg_params;

    pg_params.reserved_min = buf_pool_get_reserved_bytes(buf_pool);
    pg_params.xon_threshold = buf_pool_get_xon_threshold(buf_pool);
    pg_params.headroom = buf_pool_get_headroom_bytes(buf_pool);
    pg_params.low_limit = buf_pool_get_xoff_clear_limit(buf_pool);
    pg_params.alpha = buf_pool_get_sharing_factor(buf_pool);
    pg_params.mtu = buf_pool_get_mtu(buf_pool);
    buf_pool_get_cos_mapping(buf_pool, pg_params.cos_map, HAL_MAX_COSES, &pg_params.ncos);

    ret = capri_tm_pg_params_update(tm_port, pg, &pg_params);
    if (ret != HAL_RET_OK) {
        HAL_TRACE_ERR("PD-BUF-POOL::{}: Unable to program for buf pool: {} "
                      "Err {}",
                      __func__, buf_pool_get_buf_pool_id(buf_pool), ret);
    } else {
        HAL_TRACE_DEBUG("PD-BUF-POOL::{}: Programed for buf pool: {} ",
                      __func__, buf_pool_get_buf_pool_id(buf_pool));
    }

    return ret;
}

// ----------------------------------------------------------------------------
// Freeing Buf-Pool PD
// ----------------------------------------------------------------------------
hal_ret_t
buf_pool_pd_free (pd_buf_pool_t *buf_pool)
{
    g_hal_state_pd->buf_pool_pd_slab()->free(buf_pool);
    return HAL_RET_OK;
}

// ----------------------------------------------------------------------------
// Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
link_pi_pd(pd_buf_pool_t *pd_buf_pool, buf_pool_t *pi_buf_pool)
{
    pd_buf_pool->pi_buf_pool = pi_buf_pool;
    buf_pool_set_pd_buf_pool(pi_buf_pool, pd_buf_pool);
}

// ----------------------------------------------------------------------------
// Un-Linking PI <-> PD
// ----------------------------------------------------------------------------
void 
unlink_pi_pd(pd_buf_pool_t *pd_buf_pool, buf_pool_t *pi_buf_pool)
{
    pd_buf_pool->pi_buf_pool = NULL;
    buf_pool_set_pd_buf_pool(pi_buf_pool, NULL);
}
}    // namespace pd
}    // namespace hal
