#include "nic/include/base.h"
#include "nic/include/fte_ctx.hpp"

namespace hal {
namespace eplearn {
void dhcp_init();
bool is_dhcp_flow(const hal::flow_key_t *key);
hal_ret_t dhcp_process_packet(fte::ctx_t &ctx);
}  // namespace eplearn
}  // namespace hal
