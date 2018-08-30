# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "fte_oci",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    copts = sdk_copts,
    deps = [
        "//nic/include:base_includes",
        "//nic/hal/pd:hal_pd_includes",
        "//nic/asm/cpu-p4plus/include:cpu_p4plus_includes",
        "//nic:spdlog",
        "//nic/hal/plugins/app_redir:app_redir_includes",
        "//nic/p4/common:p4_common_includes",
        "//nic/fte:fte",
    ],
    alwayslink = 1,
)
