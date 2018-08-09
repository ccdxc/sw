# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "linkmgr_test_utils",
    srcs = [ "port_utils.cc" ],
    hdrs = [ "port_utils.hpp" ],
    deps = [
        "//nic/utils/block_list:block_list",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/hal:hal_includes",
        "//nic/hal/lib:hal_lib",
        "//nic/linkmgr:linkmgr_src",

        "//nic:libaapl",
        "//nic/linkmgr:linkmgr_includes",
        "//nic:liblinkmgr",

        "@sdk//obj:sdk_linkmgr",
        "@sdk//obj:sdk_logger",
        "@sdk//obj:sdk_thread",
        "@sdk//obj:sdk_twheel",
        "@sdk//obj:sdk_pal",
        "@sdk//obj:sdk_catalog",
        "@sdk//obj:sdk_utils",

        "//nic:spdlog",
        "//nic:grpc",
    ],
    copts = sdk_copts,
)

cc_library(
    name = "linkmgr_gtest_hdrs",
    hdrs = [ "linkmgr_base_test.hpp" ],
    deps = [
        "//:gtest",
    ],
)
