# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "nw_includes",
    srcs = glob(["*.hpp"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/hal:hal_includes",
        "//nic:gen_proto_includes",
        "//nic:hal_svc_gen_includes",
        "//nic/hal/svc:hal_svc_includes",
        "//nic:hal_gen_includes",
    ],
)

cc_library(
    name = "nw",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/hal:hal_includes",
        "//nic:gen_proto_includes",
        "//nic:hal_svc_gen_includes",
        "//nic/hal/svc:hal_svc_includes",
        "//nic:hal_gen_includes",
        "//nic/hal/periodic:hal_periodic_includes",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/utils/block_list",
        "//nic/fte:fte_includes",
        "//nic/utils/bm_allocator",
        "//nic/hal/pd:hal_pd_includes",
        "//nic:spdlog",
        "//nic/p4/iris/include:p4_iris_includes",   # TODO: session.cc including this !!
        "//nic/asm/cpu-p4plus/include:cpu_p4plus_includes",
    ],
    alwayslink = 1,
    #copts = sdk_copts,
)
