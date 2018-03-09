#include "core.hpp"
#include "nic/fte/fte.hpp"
#include "nic/hal/plugins/alg_utils/core.hpp"
#include "nic/hal/plugins/sfw/core.hpp"

namespace hal {
namespace plugins {
namespace alg_rtsp {

namespace sfw = hal::plugins::sfw;

fte::pipeline_action_t
alg_rtsp_exec(fte::ctx_t& ctx)
{
    sfw::sfw_info_t *sfw_info =
        (sfw::sfw_info_t*)ctx.feature_state(sfw::FTE_FEATURE_SFW);

    if (sfw_info->alg_proto != nwsec::APP_SVC_RTSP) {
        return fte::PIPELINE_CONTINUE;
    }

    return fte::PIPELINE_CONTINUE;
}


} // alg_rtsp
} // plugins
} // hal
