//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/include/fte_ctx.hpp"

namespace hal {
namespace eplearn {
bool dpkt_learn_required(fte::ctx_t &ctx);
hal_ret_t dpkt_learn_process_packet(fte::ctx_t &ctx);
void data_pkt_learn_init();
}  // namespace eplearn
}  // namespace hal
