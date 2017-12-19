/*
 * core.hpp
 */
#pragma once

#include "nic/include/fte.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"

namespace hal {
namespace plugins {
namespace alg_tftp {

using namespace hal::plugins::alg_utils;

/*
 * Externs
 */
extern hal::plugins::alg_utils::alg_state_t *g_tftp_state;

/*
 * Constants
 */
const std::string FTE_FEATURE_ALG_TFTP("pensando.io/alg_tftp:alg_tftp");

// TFTP opcode
#define TFTP_RRQ 0x1
#define TFTP_WRQ 0x2

/*
 * Forward declarations
 */
typedef union tftp_info_ tftp_info_t; 

/*
 * Function prototypes
 */

// plugin.cc
fte::pipeline_action_t alg_tftp_exec(fte::ctx_t &ctx);
void tftpinfo_cleanup_hdlr(l4_alg_status_t *l4_sess);
typedef hal_ret_t (*tftp_cb_t)(fte::ctx_t& ctx);

/*
 * Data Structures
 */
typedef union tftp_info_ {
    struct {  /* Expected flow info */
        uint8_t   tftpop;
        tftp_cb_t callback;
    } __PACK__;
    struct {  /* Session info */
        bool      skip_sfw;
        uint32_t  unknown_opcode;
        uint32_t  parse_errors;
    } __PACK__;
} tftp_info_t;

}  // namespace alg_tftp
}  // namespace plugins
}  // namespace hal
