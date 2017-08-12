#include "net_plugin.hpp"

namespace hal {
namespace net {

fte::pipeline_action_t
lb_exec(fte::ctx_t& ctx)
{
    HAL_TRACE_DEBUG("Invoking lb feature");
    return fte::PIPELINE_CONTINUE;
}

}
}
