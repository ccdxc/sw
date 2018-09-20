# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "telemetry",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic:spdlog",
        "//nic/hal/plugins/cfg/telemetry:telemetry_includes",
    ],
    alwayslink = 1,
    copts = sdk_copts,
)

cc_test(
    name = "telemetry_test",
    srcs = glob(["test/*.cc", "test/*.hpp"]),
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    deps = [
        "//nic/fte:fte_base_test",
	] + gl_deps_list + test_deps_list,
    copts = sdk_copts,
    tags = ["gtests"],
)

