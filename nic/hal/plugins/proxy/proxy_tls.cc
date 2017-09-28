#include "nic/hal/src/proxy.hpp"
#include "nic/hal/plugins/proxy/proxy_plugin.hpp"

namespace hal {
namespace proxy {


fte::pipeline_action_t
tls_exec(fte::ctx_t& ctx)
{
	return fte::PIPELINE_CONTINUE;
}

} // namespace hal
} // namespace net
