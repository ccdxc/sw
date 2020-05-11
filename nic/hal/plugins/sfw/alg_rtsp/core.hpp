//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include "nic/fte/fte.hpp"
#include "nic/hal/plugins/sfw/alg_utils/core.hpp"
#include "nic/hal/plugins/sfw/alg_rtsp/rtsp_parse.hpp"

#define DEBUG(args...) HAL_TRACE_DEBUG("alg_rtsp::" args)
#define ERR(args...) HAL_TRACE_ERR("alg_rtsp::" args)
#define INFO(args...) HAL_TRACE_INFO("alg_rtsp::" args)
#define WARN(args...) HAL_TRACE_WARN("alg_rtsp:: " args)

namespace alg_utils = hal::plugins::alg_utils;

using session::RTSPALGInfo;
using session::RTSPCtrlSessInfo;
using session::RTSPCtrlSessions;

namespace hal {
namespace plugins {
namespace alg_rtsp {

#define RTSP_PORT 554

/*
 * Externs
 */
extern alg_utils::alg_state_t *g_rtsp_state;
extern alg_utils::tcp_buffer_slab_t g_rtsp_tcp_buffer_slabs;

/*
 * Constants
 */
const std::string FTE_FEATURE_ALG_RTSP("pensando.io/sfw:alg_rtsp");

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
    bool               isCtrl;   // Is this a control session
    uint32_t           parse_errors; // Parse errors
};

fte::pipeline_action_t alg_rtsp_exec (fte::ctx_t &ctx);
fte::pipeline_action_t alg_rtsp_session_delete_cb (fte::ctx_t &ctx);
fte::pipeline_action_t alg_rtsp_session_get_cb (fte::ctx_t &ctx);
size_t process_control_message (void *ctxt, uint8_t *payload, size_t pkt_len);
void rtsp_app_sess_cleanup_hdlr (alg_utils::app_session_t *app_sess);
void rtsp_info_cleanup_hdlr (alg_utils::l4_alg_status_t *l4_sess);
void
add_expected_flow_from_proto(fte::ctx_t &ctx, alg_utils::app_session_t *app_sess,
                             uint32_t idle_timeout, const FlowGateKey &proto_key);

alg_utils::app_session_t *
get_rtsp_session(const rtsp_session_key_t &key, alg_utils::app_session_t *ctrl_app_sess);

hal_ret_t alg_rtsp_init(hal_cfg_t *hal_cfg);
void alg_rtsp_exit();

} // namespace alg_rtsp
} // namespace plugins
} // namespace hal
