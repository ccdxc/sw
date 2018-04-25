package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "nat",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic:spdlog",
    ],
    alwayslink = 1,
    copts = sdk_copts,
)

