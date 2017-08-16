#include <base.h>
#include <pd_api.hpp>
#include <capri_loader.h>
#include <capri_hbm.hpp>
#include <p4plus_pd_api.h>


namespace hal {
namespace pd {

hal_ret_t pd_descriptor_aol_get(pd_descr_aol_t *src, pd_descr_aol_t *dst)
{
    hal_ret_t       ret = HAL_RET_OK;

    /* TODO: Validate the src address range */
    if (!p4plus_hbm_read((uint64_t)src, (uint8_t*)dst, sizeof(pd_descr_aol_t))) {
        HAL_TRACE_ERR("Failed to read descriptor @ {}", (uint64_t) src); 
    }
    return ret;
}
}
}
