#include "net_plugin.hpp"

namespace hal {
namespace net {

void init() {
    fte::register_feature(fte::FTE_FEATURE_STAGE_MARKER, "net.stage-marker", stage_exec);
    fte::register_feature(fte::FTE_FEATURE_FWDING, "net.fwding", fwding_exec);
    fte::register_feature(fte::FTE_FEATURE_TUNNEL, "net.tunnel", tunnel_exec);
    fte::register_feature(fte::FTE_FEATURE_DFW, "net.firewall", dfw_exec);
    fte::register_feature(fte::FTE_FEATURE_LB, "net.load-balancer", lb_exec);
}

}
}
