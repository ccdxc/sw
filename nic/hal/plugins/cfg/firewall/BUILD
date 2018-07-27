# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "firewall_includes",
    srcs = glob(["*.hpp"]),
    hdrs = glob(["*.hpp"]),
    deps = [
               "//nic/hal/plugins/cfg/nw:nw_includes",
           ]
)

cc_library(
    name = "firewall",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/hal:hal_includes",
        "//nic:gen_proto_includes",
        "//nic:hal_svc_gen_includes",
        "//nic:hal_gen_includes",
        "//nic/hal/periodic:hal_periodic_includes",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/utils/block_list",
        "//nic/fte:fte_includes",
        "//nic/hal/plugins/cfg/nw:nw_includes",
        "//nic/hal/plugins/cfg/mcast:mcast_includes",
        "//nic/hal/plugins/cfg/aclqos:aclqos_includes",
        "//nic/hal/plugins/cfg/telemetry:telemetry_includes",
        "//nic/utils/bm_allocator",
        "//nic/hal/pd:hal_pd_includes",
        "//nic:spdlog",
        "//nic/hal/plugins/app_redir:app_redir_includes",
    ],
    alwayslink = 1,
    tags = [ "plugins" ]
)

cc_test(
    name = "nwsec_policy_test",
    srcs = glob(["test/*.cc"]),
    deps = [
        "//nic/hal/plugins/sfw:sfw",
        "//nic/hal/plugins/app_redir:app_redir_includes",
        "//nic/hal/plugins/alg_utils:alg_utils",
        "//nic/fte:fte_base_test",
    ] + gl_deps_list + test_deps_list,
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    tags=['gtests'],
    copts = sdk_copts,
)
