# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "control",
    srcs = [
        "hal_control.cc",
    ],
    hdrs = [
        "hal_control.hpp",
    ],
    copts = sdk_copts,
    deps = [
        "//nic:asic_includes",
        "//nic:gen_proto_includes",
        "@sdk//obj:sdk_indexer",
        "//nic/hal:hal_includes",
        "//nic/hal/lib:hal_handle_includes",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/third-party/spdlog",
        "@sdk//obj:sdk_catalog",
        "@sdk//obj:sdk_twheel",
    ],
    alwayslink = 1,
)
