#include "nic/hal/plugins/network/net_plugin.hpp"

namespace hal {
namespace net {

hal_ret_t process_tftp_first_packet(fte::ctx_t& ctx);
hal_ret_t process_tftp(fte::ctx_t& ctx);

}
}
