#include "nic/include/base.h"
#include "nic/include/fte_ctx.hpp"

namespace hal {
namespace network {
bool is_arp_flow(const hal::flow_key_t *key);
hal_ret_t arp_process_packet(fte::ctx_t &ctx);
}  // namespace network
}  // namespace hal
