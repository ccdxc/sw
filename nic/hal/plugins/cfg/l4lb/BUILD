# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "l4lb_includes",
    srcs = glob(["*.hpp"]),
    hdrs = glob(["*.hpp"]),
    deps = [],
)

cc_library(
    name = "l4lb",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/hal:hal_includes",
        "//nic:gen_proto_includes",
        "//nic:hal_svc_gen_includes",
        "//nic/hal/svc:hal_svc_includes",
        "//nic:hal_gen_includes",
        "//nic:spdlog",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/hal/core:periodic_includes",
        "//nic/fte:fte_includes",
        "//nic/utils/block_list:block_list",
        "//nic/hal/plugins/cfg/nw:nw_includes",
        "//nic/hal/plugins/sfw:firewall_includes",
        "//nic/hal/plugins/cfg/dos:dos_includes",
        "//nic/hal/plugins/cfg/aclqos:aclqos_includes",
        "//nic/hal/plugins/cfg/mcast:mcast_includes",
        "//nic/hal/plugins/cfg/telemetry:telemetry_includes",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/hal/plugins/cfg/gft:gft_includes",
        "//nic/hal/plugins/cfg/ipsec:ipsec_includes",
        #pd
        "//nic/hal/pd:hal_pd_includes",
    ],
    alwayslink = 1,
    tags = [ "plugins" ]
)
