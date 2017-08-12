#pragma once

#include "fte_ctx.hpp"

namespace fte {

extern hal_ret_t flow_init_flows(ctx_t &ctx);
extern hal_ret_t flow_update_gft(ctx_t &ctx);

} // namespace fte
