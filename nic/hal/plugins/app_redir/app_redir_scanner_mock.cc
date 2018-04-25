//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "app_redir_ctx.hpp"
#include "app_redir_scanner.hpp"

namespace hal {
namespace app_redir {

hal_ret_t scanner_init(int thread_num)
{
    return HAL_RET_OK;
}

hal_ret_t scanner_cleanup(int thread_num)
{
    return HAL_RET_OK;
}

hal_ret_t scanner_run()
{
    return HAL_RET_OK;
}

hal_ret_t scanner_run(appid_info_t& appid_info, uint8_t* pkt, uint32_t pkt_len, void* ctx)
{
    appid_info.state_ = APPID_STATE_NOT_FOUND;

    return HAL_RET_OK;
}

hal_ret_t scanner_get_appid_info(const hal::flow_key_t& key, appid_info_t& appid_info)
{
    hal::app_redir::app_redir_ctx_t::appid_info_init(appid_info);
    return HAL_RET_OK;
}

hal_ret_t scanner_cleanup_flow(void* flow_handle) {
    return HAL_RET_OK;
}

} // namespace app_redir
} // namespace hal
