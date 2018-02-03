# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "gft",
    srcs = [] + glob(["*.cc"]),
    hdrs = [] + glob(["*.hpp"]),
    deps = [
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
        "@sdk//:sdk_headers",
        "@sdk//obj:sdk_tcam",
        "@sdk//obj:sdk_hash",
        "@sdk//obj:sdk_directmap",
    ],
    copts = sdk_copts,
    alwayslink = 1,
)
