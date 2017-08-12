#include "net_plugin.hpp"

namespace hal {
namespace net {

fte::pipeline_action_t
dfw_exec(fte::ctx_t& ctx)
{
    HAL_TRACE_DEBUG("Invoking firewall feature");
    return fte::PIPELINE_CONTINUE;
}

}
}
