# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "linkmgr_test_deps_list")

cc_test(
    name = "port_test",
    srcs = ["port_test.cc"],
    deps = [
        "//nic/linkmgr:linkmgr_src"
    ] + linkmgr_test_deps_list,
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-rdynamic",
        "-ldl",
        "-lm",
        "-lrt",
    ],
    copts = sdk_copts,
)
