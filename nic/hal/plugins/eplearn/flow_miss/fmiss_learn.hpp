//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/include/base.hpp"
#include "nic/include/fte_ctx.hpp"

namespace hal {
namespace eplearn {
bool flow_miss_learning_required(fte::ctx_t &ctx);
hal_ret_t flow_miss_process_packet(fte::ctx_t &ctx);
void flow_miss_learn_init();
}  // namespace eplearn
}  // namespace hal
