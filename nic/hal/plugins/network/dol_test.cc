#include "net_plugin.hpp"
#include <stdint.h>
#include "nic/model_sim/include/lib_model_client.h"

namespace hal {
namespace net {

fte::pipeline_action_t
dol_test_exec(fte::ctx_t& ctx)
{
    if (ctx.pkt_len()) {
        step_cpu_pkt(ctx.pkt(), ctx.pkt_len());
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace net
} // namespace hal
