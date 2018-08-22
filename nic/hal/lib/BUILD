# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "hal_lib_includes",
    hdrs = [
        "hal_handle.hpp",
    ],
    deps = [
        "@sdk//obj:sdk_list",
    ]
)

cc_library(
    name = "hal_lib",
    srcs = [
        "hal_handle.cc",
    ],
    hdrs = [
        "hal_handle.hpp",
    ],
    deps = [
        "@sdk//obj:sdk_list",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_ht",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic:boost",
        "@sdk//obj:sdk_periodic",
        "//nic/fte:fte_includes"
    ],
    copts = sdk_copts,
)

