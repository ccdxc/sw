# {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#
package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "lif_includes",
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/utils/bm_allocator:bm_allocator",
    ],
)

cc_library(
    name = "lif",
    srcs = glob(["*.cc"]),
    deps = [
        "//nic/hal:hal_includes",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/hal/plugins/cfg/lif:lif_includes",
        "//nic/hal/plugins/cfg/mcast:mcast_includes",
        "//nic/hal/plugins/cfg/telemetry:telemetry_includes",
        "//nic/hal/pd:hal_pd_includes",
        "//nic/fte:fte_includes",
    ],
    alwayslink = 1,
    tags = [ "plugins" ]
)
