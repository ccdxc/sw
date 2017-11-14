#include "nic/hal/plugins/network/net_plugin.hpp"

extern "C" void __plugin_init() {
    hal::net::init();
}

namespace hal {
namespace net {

void init() {
    fte::register_feature(fte::FTE_FEATURE_STAGE_MARKER, "net.stage-marker", stage_exec);
    fte::register_feature(fte::FTE_FEATURE_FWDING, "net.fwding", fwding_exec);
    fte::register_feature(fte::FTE_FEATURE_TUNNEL, "net.tunnel", tunnel_exec);
    fte::register_feature(fte::FTE_FEATURE_DFW, "net.firewall", dfw_exec);
    fte::register_feature(fte::FTE_FEATURE_LB, "net.load-balancer", lb_exec);
    fte::register_feature(fte::FTE_FEATURE_QOS, "net.qos", qos_exec);
    fte::register_feature(fte::FTE_FEATURE_DOL_TEST_ONLY, "net.doltest", dol_test_exec);
    fte::register_feature(fte::FTE_FEATURE_INGRESS_CHECKS, "net.ingress-checks", ingress_checks_exec);
}

}
}
