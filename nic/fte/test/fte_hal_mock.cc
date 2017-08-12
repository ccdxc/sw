#include <base.h>
#include <session.hpp>
#include <pd_api.hpp>

hal::hal_state *hal::g_hal_state = hal::hal_state::factory();

hal::hal_state::hal_state(){}

hal::hal_state *
hal::hal_state::factory() {
    return new hal::hal_state();
}

hal_ret_t
hal::ep_get_from_flow_key(const hal::flow_key_t*, hal::ep_t**, hal::ep_t**)
{
    return HAL_RET_EP_NOT_FOUND;
}

const char *
hal::flowkey2str(const hal::flow_key_t&)
{
    return "unsupported";
}

hal_ret_t
hal::session_create(const hal::session_args_s *, hal_handle_t*)
{
    return HAL_RET_OK;
}

hal::l2seg_t *
hal::pd::find_l2seg_by_hwid(hal::pd::l2seg_hw_id_t)
{
    return NULL;
}

