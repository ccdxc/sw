#include "proxy_plugin.hpp"
#include "app_redir.hpp"

namespace hal {
namespace proxy {

void init() {
    fte::register_feature(fte::FTE_FEATURE_TCP_PROXY, "proxy.tcp", tcp_exec);
    fte::register_feature(fte::FTE_FEATURE_TLS_PROXY, "proxy.tls", tls_exec);
    fte::register_feature(fte::FTE_FEATURE_IPSEC, "proxy.ipsec", ipsec_exec);
    fte::register_feature(fte::FTE_FEATURE_APP_REDIR_MISS, "app.redirect.miss", app_redir_miss_exec);
    fte::register_feature(fte::FTE_FEATURE_APP_REDIR, "app.redirect", app_redir_exec);
    fte::register_feature(fte::FTE_FEATURE_APP_REDIR_FINI, "app.redirect.fini", app_redir_exec_fini);
}

}
}
