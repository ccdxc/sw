#pragma once

#include <fte.hpp>

namespace hal {
namespace net {

void init();

fte::pipeline_action_t fwding_exec(fte::ctx_t &ctx);
fte::pipeline_action_t dfw_exec(fte::ctx_t &ctx);
fte::pipeline_action_t lb_exec(fte::ctx_t &ctx);

}
}
