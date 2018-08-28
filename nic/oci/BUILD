# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")

cc_binary(
    name = "oci",
    srcs = [
        "main.cc",
    ],
    copts = sdk_copts,
    linkopts = [
        "-lpthread",
        "-pthread",
        "-lz",
        "-rdynamic",
        "-lm",
        "-lrt",
        "-ldl",
    ],
    deps = [
            ":hal",
            "//nic/utils/mtrack",
    ],
)

cc_library(
    name = "hal_includes",
    hdrs = [
        "hal.hpp",
        "hal_state.hpp",
    ]
)

cc_library(
    name = "hal",
    srcs = [
        "hal.cc",
        "hal_state.cc",
    ],
    copts = sdk_copts,
    deps = [
        "@sdk//obj:sdk_logger",
        "@sdk//obj:sdk_thread",
        "@sdk//obj:sdk_catalog",
        "@sdk//obj:sdk_utils",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_twheel",
        "//nic:spdlog",
        "//nic/include:common_includes",
        "//nic/oci:hal_includes",
        "//nic/fte:fte_core_includes",
        "//nic/hal/core:core",
        "//nic/fte:fte",
        "//nic/fte/oci:fte_oci",
        "//nic/utils/print",

        # linkmgr dependencies
        "//nic/linkmgr:linkmgr_includes",
        "//nic:liblinkmgr",
        "@sdk//obj:sdk_linkmgr",
        "//nic:libaapl",
    ],
)
