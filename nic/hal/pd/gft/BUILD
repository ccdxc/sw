# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "gft",
    srcs = [] + glob(["*.cc"]),
    hdrs = [] + glob(["*.hpp"]) + glob(["*.h"]),
    deps = [
        "//nic/include:base_includes",
        "//nic:spdlog",
    ],
    copts = sdk_copts,
    alwayslink = 1,
)
