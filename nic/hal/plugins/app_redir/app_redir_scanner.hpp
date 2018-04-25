//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include "nic/include/fte_ctx.hpp"
#include "app_redir_ctx.hpp"

namespace hal {
namespace app_redir {

hal_ret_t scanner_init(int thread_num);
hal_ret_t scanner_cleanup(int thread_num);
hal_ret_t scanner_run();
hal_ret_t scanner_run(appid_info_t& appid_info, uint8_t* pkt, uint32_t pkt_len, void* ctx);
hal_ret_t scanner_get_appid_info(const hal::flow_key_t& key, appid_info_t& appid_info);
hal_ret_t scanner_cleanup_flow(void* flow_handle);

} // namespace app_redir
} // namespace hal
