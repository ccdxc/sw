/*
 * core.hpp
 */
#pragma once

#include "nic/fte/fte.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"
#include "nic/hal/plugins/alg_rtsp/rtsp_parse.hpp"

#define DEBUG(args...) HAL_TRACE_DEBUG("alg_rtsp::" args)
#define ERR(args...) HAL_TRACE_ERR("alg_rtsp::" args)
#define INFO(args...) HAL_TRACE_INFO("alg_rtsp::" args)
#define WARN(args...) HAL_TRACE_WARN("alg_rtsp:: " args)

namespace alg_utils = hal::plugins::alg_utils;

namespace hal {
namespace plugins {
namespace alg_rtsp {

#define RTSP_PORT 554

/*
 * Externs
 */
extern alg_utils::alg_state_t *g_rtsp_state;

/*
 * Constants
 */
const std::string FTE_FEATURE_ALG_RTSP("pensando.io/alg_rtsp:alg_rtsp");

struct rtsp_session_key_t {
    vrf_id_t          vrf_id;     
    ip_addr_t         ip;       // server ip
    uint16_t          port;     // server control port
    rtsp_session_id_t id;       // session id    
};

// spdlog formatter for session_key_t
inline std::ostream& operator<<(std::ostream& os, const rtsp_session_key_t& key)
{
    return os << "{vrf=" << key.vrf_id
              << ", ip=" << ipaddr2str(&key.ip)
              << ", port=" << key.port
              << ", id=" << key.id
              << "}";
}

struct rtsp_session_t {
    rtsp_session_key_t sess_key;
    uint32_t           timeout;  // session timeout
};

fte::pipeline_action_t alg_rtsp_exec(fte::ctx_t &ctx);
fte::pipeline_action_t alg_ftp_session_delete_cb(fte::ctx_t &ctx);

} // namespace alg_rtsp
} // namespace plugins
} // namespace hal
