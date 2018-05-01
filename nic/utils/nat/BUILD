# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "nat",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic:hal_gen_includes",
        "//nic/hal:hal_includes",
        "//nic/hal/lib:hal_lib",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_slab",
    ],
)
