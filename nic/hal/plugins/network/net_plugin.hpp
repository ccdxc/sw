#pragma once

#include "nic/include/fte.hpp"

namespace hal {
namespace net {

void init();

fte::pipeline_action_t stage_exec(fte::ctx_t &ctx);
fte::pipeline_action_t tunnel_exec(fte::ctx_t &ctx);
fte::pipeline_action_t fwding_exec(fte::ctx_t &ctx);
fte::pipeline_action_t dfw_exec(fte::ctx_t &ctx);
fte::pipeline_action_t lb_exec(fte::ctx_t &ctx);
fte::pipeline_action_t qos_exec(fte::ctx_t &ctx);
fte::pipeline_action_t dol_test_exec(fte::ctx_t &ctx);
fte::pipeline_action_t ep_learn_exec(fte::ctx_t &ctx);
fte::pipeline_action_t ingress_checks_exec(fte::ctx_t &ctx);
}
}
