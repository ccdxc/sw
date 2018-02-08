# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_binary(
    name = "libgft.so",
    linkshared = 1,
    deps = [
        "//nic/hal/pd/gft:pd_gft"
    ],
)

cc_library(
    name = "pd_gft",
    srcs = [] + glob(["*.cc"]),
    hdrs = [] + glob(["*.hpp"]),
    deps = [
        "//nic:gft_p4pd",
        "//nic:p4pluspd_txdma",
        "//nic:p4pluspd_rxdma",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic/hal:hal_hdrs",
        "//nic/utils/block_list:block_list",
        "//nic/hal/periodic:periodic",
        "//nic:gen_proto_includes",
        "//nic/utils/bm_allocator",
        "//nic/hal/pd:hal_pd_includes",
        "//nic:gen_includes",
        "//nic/hal/pd/utils/flow",
        "//nic/hal/pd/p4pd:p4pd",
        "//nic/p4/gft/include:p4_gft_includes",
        "//nic/hal/pd/capri",
        "//nic/hal/pd/globalpd",
        "@sdk//:sdk_headers",
        "@sdk//obj:sdk_tcam",
        "@sdk//obj:sdk_hash",
        "@sdk//obj:sdk_directmap",
    ],
    copts = sdk_copts,
    alwayslink = 1,
)
