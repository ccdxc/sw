package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "plugins",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    copts = sdk_copts,
    deps = [
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic:spdlog",
    ],
    alwayslink = 1,
)
