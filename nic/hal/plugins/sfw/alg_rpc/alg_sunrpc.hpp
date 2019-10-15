//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include "core.hpp"
#include "nic/include/fte.hpp"

namespace hal {
namespace plugins {
namespace alg_rpc {

using namespace hal::plugins::alg_utils;

/*
 * Function prototypes
 */
size_t parse_sunrpc_control_flow(void *ctxt, uint8_t *pkt, size_t pkt_len);
size_t process_sunrpc_data_flow(void *ctxt, uint8_t *pkt, size_t pkt_len);

}  // namespace alg_rpc
}  // namespace plugins
}  // namespace hal
