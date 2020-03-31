#include "nic/include/base.hpp"
#include "nic/include/pd_api.hpp"
#include "nic/sdk/asic/cmn/asic_hbm.hpp"
#include "nic/hal/pd/iris/internal/p4plus_pd_api.h"


namespace hal {
namespace pd {

hal_ret_t
 // pd_descriptor_aol_get(pd_descr_aol_t *src, pd_descr_aol_t *dst)
pd_descriptor_aol_get(pd_func_args_t *pd_func_args)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_descriptor_aol_get_args_t *args = pd_func_args->pd_descriptor_aol_get;
    pd_descr_aol_t *src = args->src;
    pd_descr_aol_t *dst = args->dst;

    /* TODO: Validate the src address range */
    if (sdk::asic::asic_mem_read((uint64_t)src, (uint8_t*)dst, sizeof(pd_descr_aol_t))) {
        HAL_TRACE_ERR("Failed to read descriptor @ {}", (uint64_t) src);
    }
    return ret;
}
}
}
