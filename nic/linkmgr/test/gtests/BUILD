# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_test(
    name = "port_test",
    srcs = ["port_test.cc"],
    deps = [
        "//nic/linkmgr/test/utils:linkmgr_gtest_hdrs",
        "//nic/linkmgr/test/utils:linkmgr_test_utils",
    ],
    linkopts = [
        "-ldl",
        "-lJudy",
    ],
    copts = sdk_copts,
)
