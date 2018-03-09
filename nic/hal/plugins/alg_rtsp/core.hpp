/*
 * core.hpp
 */
#pragma once

#include "nic/fte/fte.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"

#define DEBUG(args...) HAL_TRACE_DEBUG("alg_rtsp::" args)
#define ERR(args...) HAL_TRACE_ERR("alg_rtsp::" args)
#define INFO(args...) HAL_TRACE_INFO("alg_rtsp::" args)
#define WARN(args...) HAL_TRACE_WARN("alg_rtsp:: " args)

namespace alg_utils = hal::plugins::alg_utils;

namespace hal {
namespace plugins {
namespace alg_rtsp {

/*
 * Externs
 */
extern alg_utils::alg_state_t *g_rtsp_state;

typedef struct rtsp_info_ {
} rtsp_info_t;

fte::pipeline_action_t alg_rtsp_exec(fte::ctx_t &ctx);

} // namespace alg_rtsp
} // namespace plugins
} // namespace hal
