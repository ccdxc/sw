#include "proxy_plugin.hpp"

namespace hal {
namespace proxy {

void init() {
    fte::register_feature(fte::FTE_FEATURE_TCP_PROXY, "proxy.tcp", tcp_exec);
    fte::register_feature(fte::FTE_FEATURE_TLS_PROXY, "proxy.tls", tls_exec);
}

}
}
