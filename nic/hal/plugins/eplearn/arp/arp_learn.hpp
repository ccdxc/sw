//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.h"
#include "nic/include/fte_ctx.hpp"

namespace hal {
namespace eplearn {
bool is_arp_flow(const hal::flow_key_t *key);
bool is_arp_broadcast(const hal::flow_key_t *key);
hal_ret_t arp_process_packet(fte::ctx_t &ctx);
}  // namespace eplearn
}  // namespace hal
