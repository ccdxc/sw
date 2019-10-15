//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include "nic/include/fte.hpp"
#include "nic/hal/plugins/sfw/alg_utils/core.hpp"

namespace hal {
namespace plugins {
namespace alg_tftp {

using namespace hal::plugins::alg_utils;

/*
 * Externs
 */
extern alg_state_t *g_tftp_state;

/*
 * Constants
 */
const std::string FTE_FEATURE_ALG_TFTP("pensando.io/sfw:alg_tftp");

// TFTP opcode
#define TFTP_RRQ 0x1
#define TFTP_WRQ 0x2

/*
 * Forward declarations
 */
typedef struct tftp_info_ tftp_info_t;

/*
 * Function prototypes
 */

// plugin.cc
fte::pipeline_action_t alg_tftp_exec(fte::ctx_t &ctx);
void tftpinfo_cleanup_hdlr(l4_alg_status_t *l4_sess);
typedef hal_ret_t (*tftp_cb_t)(fte::ctx_t& ctx, l4_alg_status_t *exp_flow);
fte::pipeline_action_t alg_tftp_session_delete_cb(fte::ctx_t &ctx);
fte::pipeline_action_t alg_tftp_session_get_cb(fte::ctx_t &ctx);
hal_ret_t process_tftp(fte::ctx_t& ctx, l4_alg_status_t *l4_sess);
hal_ret_t alg_tftp_init(hal_cfg_t *hal_cfg);
void alg_tftp_exit();

/*
 * Data Structures
 */
typedef struct tftp_info_ {
    uint32_t  unknown_opcode;
    uint32_t  parse_errors;
    tftp_cb_t callback;
    bool      skip_sfw;
    uint16_t   tftpop;
} tftp_info_t;

}  // namespace alg_tftp
}  // namespace plugins
}  // namespace hal
