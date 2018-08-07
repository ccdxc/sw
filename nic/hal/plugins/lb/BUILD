package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "lb",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic/hal/plugins/cfg/l4lb:l4lb_includes",
    ],
    copts = sdk_copts,
    alwayslink = 1,
)

