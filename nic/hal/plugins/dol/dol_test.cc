#include <stdint.h>
#include "nic/include/fte.hpp"
#include "nic/include/asic_pd.hpp"

namespace hal {
namespace dol {

thread_local bool g_handler_registered;

static void
completion_handler(fte::ctx_t& ctx, bool fail)
{
    hal::pd::asic_step_cpu_pkt(ctx.pkt(), ctx.pkt_len());
    g_handler_registered = false;
}

fte::pipeline_action_t
dol_test_exec(fte::ctx_t& ctx)
{
    if (ctx.pkt() != NULL && !g_handler_registered) {
        ctx.register_completion_handler(completion_handler);
        g_handler_registered = true;
    }
    return fte::PIPELINE_CONTINUE;
}

} // namespace dol
} // namespace hal
