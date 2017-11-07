#include "nic/hal/plugins/network/net_plugin.hpp"

namespace hal {
namespace net {

hal_ret_t parse_sunrpc_control_flow(fte::ctx_t& ctx);
hal_ret_t process_sunrpc_control_flow(fte::ctx_t& ctx);
hal_ret_t process_sunrpc_data_flow(fte::ctx_t& ctx);
hal_ret_t parse_msrpc_control_flow(fte::ctx_t& ctx);
hal_ret_t process_msrpc_control_flow(fte::ctx_t& ctx);
hal_ret_t process_msrpc_data_flow(fte::ctx_t& ctx);

}
}
