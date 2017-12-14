package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "control",
    srcs = [
    ] + glob(["*.cc"]),
    hdrs = [
    ] + glob(["*.hpp"]) + glob(["*.h"]),
    copts = sdk_copts,
    deps = [
        "//nic:asic_includes",
        "//nic:gen_proto_includes",
        "//nic:sdk_indexer",
        "//nic/hal:hal_includes",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/catalog",
        "//nic/utils/thread",
        "//nic/utils/twheel",
    ],
    alwayslink = 1,
)
