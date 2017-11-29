
#include "app_redir_scanner.hpp"

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

hal_ret_t scanner_run(fte::appid_info_t& appid_info, uint8_t* pkt, uint32_t pkt_len, void* ctx)
{
    appid_info.state_ = hal::APPID_STATE_NOT_FOUND;

    return HAL_RET_OK;
}

hal_ret_t scanner_get_appid_info(const hal::flow_key_t& key, fte::appid_info_t& appid_info)
{
    appid_info_init(appid_info);
    return HAL_RET_OK;
}

hal_ret_t scanner_cleanup_flow(void* flow_handle) {
    return HAL_RET_OK;
}
