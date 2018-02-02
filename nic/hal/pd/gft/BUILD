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
    ],
    copts = sdk_copts,
    alwayslink = 1,
)
