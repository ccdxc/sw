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
hal::session_create_fte(const hal::session_args_fte_t *, hal_handle_t*)
{
    return HAL_RET_OK;
}

hal::l2seg_t *
hal::pd::find_l2seg_by_hwid(hal::pd::l2seg_hw_id_t)
{
    return NULL;
}

hal_ret_t
hal::qos_extract_action_from_spec(hal::qos_action_s*, qos::QOSActions const&, hal::qos_direction_e)
{
    return HAL_RET_OK;
}

uint32_t
hal::pd::ep_pd_get_rw_tbl_idx_from_pi_ep(hal::ep_t*, rewrite_actions_en)
{
    return 0;
}
uint32_t
hal::pd::ep_pd_get_tnnl_rw_tbl_idx_from_pi_ep(hal::ep_t*, 
                                              tunnel_rewrite_actions_en)
{
    return 0;
}

hal_ret_t
hal::pd::pd_rw_entry_find_or_alloc(hal::pd::pd_rw_entry_args_t*, unsigned int*)
{
    return HAL_RET_OK;
}

mac_addr_t *
hal::ep_get_mac_addr(hal::ep_s*)
{
    return NULL;
}

void *
hal::hal_handle::get_obj(void)
{
    return NULL;
}

hal_ret_t
hal::hal_cfg_db_open(hal::cfg_op_e)
{
    return HAL_RET_OK;
}

hal_ret_t
hal::hal_cfg_db_close(bool)
{
    return HAL_RET_OK;
}
