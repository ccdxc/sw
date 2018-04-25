//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "core.hpp"

namespace hal {
namespace plugins {
namespace alg_sip {

fte::pipeline_action_t alg_sip_exec(fte::ctx_t &ctx) {
    return fte::PIPELINE_CONTINUE;
}

}  // namespace alg_sip
}  // namespace plugins
}  // namespace hal
