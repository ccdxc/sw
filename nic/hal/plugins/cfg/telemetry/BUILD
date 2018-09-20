# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "telemetry_includes",
    srcs = glob(["*.hpp"]),
    hdrs = glob(["*.hpp"]),
    deps = [],
)

cc_library(
    name = "telemetrycfg",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/hal:hal_includes",
        "//nic:gen_proto_includes",
        "//nic:hal_svc_gen_includes",
        "//nic:hal_gen_includes",
        "@sdk//obj:sdk_periodic",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/utils/block_list",
        "//nic/fte:fte_includes",
        "//nic/utils/bm_allocator",
        "//nic/hal/pd:hal_pd_includes",
        "//nic/hal/plugins/cfg/gft:gft_includes",
        "//nic/hal/plugins/cfg/mcast:mcast_includes",
        "//nic/hal/plugins/cfg/ipsec:ipsec_includes",
        "//nic/hal/plugins/cfg/lif:lif_includes",
        "//nic:spdlog",
    ],
    alwayslink = 1,
    tags = [ "plugins" ]
)
