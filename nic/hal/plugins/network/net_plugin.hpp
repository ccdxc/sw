//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#pragma once

#include "nic/include/fte.hpp"

namespace hal {
namespace plugins {
namespace network {

void init();

fte::pipeline_action_t stage_exec(fte::ctx_t &ctx);
fte::pipeline_action_t tunnel_exec(fte::ctx_t &ctx);
fte::pipeline_action_t fwding_pre_stage_exec(fte::ctx_t &ctx);
fte::pipeline_action_t fwding_exec(fte::ctx_t &ctx);
fte::pipeline_action_t dfw_exec(fte::ctx_t &ctx);
fte::pipeline_action_t lb_exec(fte::ctx_t &ctx);
fte::pipeline_action_t qos_exec(fte::ctx_t &ctx);
fte::pipeline_action_t dol_test_exec(fte::ctx_t &ctx);
fte::pipeline_action_t ingress_checks_exec(fte::ctx_t &ctx);


} // namespace network
} // namespace plugins
} // namespace hal
