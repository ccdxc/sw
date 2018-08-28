# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "fte_iris",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    copts = sdk_copts,
    deps = [
        "//nic/include:base_includes",
        "//nic/hal/pd:hal_pd_includes",
        "//nic/asm/cpu-p4plus/include:cpu_p4plus_includes",
        "//nic:spdlog",
        "//nic/hal/plugins/app_redir:app_redir_includes",
        "//nic/utils/agent_api:agent_api",
        "//nic/hal/iris/datapath/p4/include:p4_iris_includes",
        "//nic/p4/common:p4_common_includes",
        "//nic/fte:fte",
    ],
    alwayslink = 1,
)
