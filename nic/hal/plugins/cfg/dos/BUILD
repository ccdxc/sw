# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "dos_includes",
    srcs = glob(["*.hpp"]),
    hdrs = glob(["*.hpp"]),
    deps = [],
)

cc_library(
    name = "dos",
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
        "@sdk//obj:sdk_periodic",
        "//nic/fte:fte_includes",
        "//nic/hal/plugins/cfg/nw:nw_includes",
        "//nic/utils/block_list:block_list",
        "//nic/hal/plugins/sfw:firewall_includes",
        "//nic/hal/plugins/cfg/aclqos:aclqos_includes",
        "//nic/hal/plugins/cfg/mcast:mcast_includes",
        "//nic/hal/plugins/cfg/telemetry:telemetry_includes",
        "//nic/hal/plugins/cfg/l4lb:l4lb_includes",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/hal/plugins/cfg/gft:gft_includes",
        "//nic/hal/plugins/cfg/ipsec:ipsec_includes",
        #pd
        "//nic/hal/pd:hal_pd_includes",
    ],
    alwayslink = 1,
    tags = [ "plugins" ]
)
