# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "linkmgr_test_utils",
    srcs = [ "port_utils.cc" ],
    hdrs = [ "port_utils.hpp" ],
    deps = [
        "//nic/linkmgr:linkmgr_src",
    ],
    copts = sdk_copts
)

cc_library(
    name = "linkmgr_gtest_utils",
    hdrs = [ "linkmgr_base_test.hpp" ],
    deps = [
        ":linkmgr_test_utils",
    ],
    copts = sdk_copts
)
