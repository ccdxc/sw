# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "gft_includes",
    srcs = glob(["*.hpp"]),
    hdrs = glob(["*.hpp"]),
    deps = [],
)

cc_library(
    name = "gft",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/hal:hal_includes",
        "//nic:gen_proto_includes",
        "//nic:hal_svc_gen_includes",
        "//nic:hal_gen_includes",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/hal/pd:hal_pd_includes",
        "//nic/utils/block_list",
        "//nic/fte:fte_includes",
        "//nic/utils/bm_allocator",
        "//nic/hal/plugins/cfg/mcast:mcast_includes",
	    "//nic/hal/plugins/cfg/ipsec:ipsec_includes",
        "//nic/hal/plugins/cfg/telemetry:telemetry_includes",
        "//nic:spdlog",
    ],
    alwayslink = 1,
    tags = [ "plugins" ]
)
