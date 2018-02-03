#include "nic/include/base.h"
#include "nic/include/pd_api.hpp"
#include "nic/hal/pd/capri/capri_loader.h"
#include "nic/hal/pd/capri/capri_hbm.hpp"
#include "nic/hal/pd/iris/p4plus_pd_api.h"


namespace hal {
namespace pd {

hal_ret_t 
 // pd_descriptor_aol_get(pd_descr_aol_t *src, pd_descr_aol_t *dst)
pd_descriptor_aol_get(pd_descriptor_aol_get_args_t *args)
{
    hal_ret_t       ret = HAL_RET_OK;
    pd_descr_aol_t *src = args->src;
    pd_descr_aol_t *dst = args->dst;

    /* TODO: Validate the src address range */
    if (!p4plus_hbm_read((uint64_t)src, (uint8_t*)dst, sizeof(pd_descr_aol_t))) {
        HAL_TRACE_ERR("Failed to read descriptor @ {}", (uint64_t) src); 
    }
    return ret;
}
}
}
