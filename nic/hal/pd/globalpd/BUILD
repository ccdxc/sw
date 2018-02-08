# {C} Copyright 2017 Pensando Systems Inc. All rights reserved
#
# - Global PD.
#   - Commone Code for all PDs

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "globalpd",
    srcs = glob(["*.cc"]),
    hdrs = [
        "gpd_utils.hpp",
    ],
    deps = [
        "//nic/include:base_includes",
        "//nic:spdlog",
    ]
)
