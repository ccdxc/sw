#include "proxy_plugin.hpp"
#include "app_redir.hpp"

namespace hal {
namespace proxy {

const std::string FTE_FEATURE_TCP_PROXY("pensando.io/proxy:tcp");
const std::string FTE_FEATURE_TLS_PROXY("pensando.io/proxy:tls");
const std::string FTE_FEATURE_IPSEC("pensando.io/proxy:ipsec");
const std::string FTE_FEATURE_QUIESCE("pensando.io/proxy:quiesce");
const std::string FTE_FEATURE_APP_REDIR_MISS("pensando.io/proxy:app-redir-miss");
const std::string FTE_FEATURE_APP_REDIR("pensando.io/proxy:app-redir");
const std::string FTE_FEATURE_APP_REDIR_FINI("pensando.io/proxy:app-redir-fini");
const std::string FTE_FEATURE_P4PT("pensando.io/proxy:app-p4pt");

extern "C" hal_ret_t proxy_plugin_init() {
    fte::register_feature(FTE_FEATURE_TCP_PROXY, tcp_exec);
    fte::register_feature(FTE_FEATURE_QUIESCE, quiesce_exec);
    fte::register_feature(FTE_FEATURE_TLS_PROXY, tls_exec);
    fte::register_feature(FTE_FEATURE_IPSEC, ipsec_exec);
    fte::register_feature(FTE_FEATURE_APP_REDIR_MISS, app_redir_miss_exec);
    fte::register_feature(FTE_FEATURE_APP_REDIR, app_redir_exec);
    fte::register_feature(FTE_FEATURE_APP_REDIR_FINI, app_redir_exec_fini);
    fte::register_feature(FTE_FEATURE_P4PT, p4pt_exec);
    return HAL_RET_OK;
}

extern "C" void proxy_exit() {
}

}
}
