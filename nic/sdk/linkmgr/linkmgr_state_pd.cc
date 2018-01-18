
#include "linkmgr_state_pd.hpp"
#include "nic/include/hal_mem.hpp"

namespace linkmgr {
namespace pd {

//------------------------------------------------------------------------------
// factory method
//------------------------------------------------------------------------------
linkmgr_state_pd *
linkmgr_state_pd::factory(void)
{
    linkmgr_state_pd *state = NULL;

    state = new linkmgr_state_pd();
    if (NULL == state) {
        return NULL;
    }

    if (state->init() != SDK_RET_OK) {
        delete state;
        return NULL;
    }

    return state;
}

sdk_ret_t
linkmgr_state_pd::init(void)
{
    sdk_ret_t ret = SDK_RET_OK;

    port_slab_ = slab::factory("port_pd", hal::HAL_SLAB_PORT_PD,
                               sizeof(linkmgr::pd::port), 8,
                               false, true, true);

    if (NULL == port_slab_) {
        ret = SDK_RET_ERR;
    }

    return ret;
}

} /* pd */
} /* linkmgr */
