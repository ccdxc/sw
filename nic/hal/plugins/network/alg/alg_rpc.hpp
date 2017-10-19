#pragma once

#include "nic/hal/src/session.hpp"
#include "nic/include/fte_db.hpp"
#include "nic/hal/plugins/network/net_plugin.hpp"
#include "rpc/rpc.h"

#define RPC_XID_WIDTH 4

namespace hal {
namespace net {

hal_ret_t process_rpc_control_flow(fte::ctx_t& ctx);
hal_ret_t parse_rpc_control_flow(fte::ctx_t& ctx);

}
}
