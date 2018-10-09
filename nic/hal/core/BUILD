# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "core_includes",
    srcs = [],
    hdrs = [
        "core.hpp",
        "plugins.hpp",
    ],
    deps = [
        "@sdk//obj:sdk_thread",
    ]
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
        "//nic/fte:fte_core_includes",
        "@sdk//obj:sdk_periodic",
        "//nic/hal/core:core_includes",
    ],
)

cc_library(
    name = "heartbeat_includes",
    srcs = [],
    hdrs = glob(["heartbeat/heartbeat.hpp"]),
)

cc_library(
    name = "heartbeat",
    srcs = [
        "heartbeat/heartbeat.cc",
    ],
    hdrs = ["heartbeat/heartbeat.hpp"],
    deps = [
        "//nic/include:base_includes",
        "@sdk//obj:sdk_periodic",
        "//nic/hal/core:heartbeat_includes",
        "@sdk//:sdk_headers",
        "//nic/hal/core:core_includes",
        "//nic:spdlog",
    ],
)

