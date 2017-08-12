#include "net_plugin.hpp"

namespace hal {
namespace net {

fte::pipeline_action_t
fwding_exec(fte::ctx_t& ctx)
{
    HAL_TRACE_DEBUG("Invoking fwding feature");
    return fte::PIPELINE_CONTINUE;
}

} // namespace hal 
} // namespace net 
