//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include "nic/include/fte.hpp"
#include "nic/hal/plugins/sfw/alg_utils/core.hpp"

namespace hal {
namespace plugins {
namespace alg_ftp {

using namespace hal::plugins::alg_utils;
using session::FTPALGInfo;
using session::FTPCmdType;

/*
 * Externs
 */
extern alg_state_t *g_ftp_state;
extern tcp_buffer_slab_t g_ftp_tcp_buffer_slabs;

/*
 * Constants
 */
const std::string FTE_FEATURE_ALG_FTP("pensando.io/sfw:alg_ftp");

/*
 * Function prototypes
 */

// plugin.cc
fte::pipeline_action_t alg_ftp_exec(fte::ctx_t &ctx);
fte::pipeline_action_t alg_ftp_session_delete_cb(fte::ctx_t &ctx);
fte::pipeline_action_t alg_ftp_session_get_cb(fte::ctx_t &ctx);

hal_ret_t alg_ftp_init(hal_cfg_t *hal_cfg);
void alg_ftp_exit();
void ftpinfo_cleanup_hdlr(l4_alg_status_t *l4_sess);
void
add_expected_flow_from_proto (fte::ctx_t &ctx, l4_alg_status_t *l4_sess, const FlowGateKey &proto_key);

}  // namespace alg_ftp
}  // namespace plugins
}  // namespace hal
