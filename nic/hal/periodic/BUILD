# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "periodic",
    srcs = [
        "periodic.cc",
    ],
    hdrs = ["periodic.hpp"],
    deps = [
        "//nic/hal:hal_includes",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_thread",
        "@sdk//obj:sdk_twheel",
        "@sdk//obj:sdk_timerfd",
    ],
)
