# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "periodic_includes",
    srcs = [],
    hdrs = glob(["periodic/periodic.hpp"]),
)

cc_library(
    name = "periodic",
    srcs = [
        "periodic/periodic.cc",
    ],
    hdrs = ["periodic/periodic.hpp"],
    deps = [
        "//nic/include:base_includes",
        "//nic:spdlog",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_thread",
        "@sdk//obj:sdk_twheel",
        "@sdk//obj:sdk_timerfd",
    ],
)

cc_library(
    name = "core_includes",
    srcs = [],
    hdrs = [
        "init.hpp",
        "plugins.hpp",
    ],
    deps = [],
)

cc_library(
    name = "core",
    srcs = [
        "init.cc",
        "plugins.cc",
    ],
    hdrs = [],
    deps = [
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic/fte:fte_includes",
        "@sdk//obj:sdk_thread",
        "//nic/hal/core:periodic_includes",
        "//nic/hal/core:core_includes",
    ],
)
