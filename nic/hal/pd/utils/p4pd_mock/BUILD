package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "p4pd_mock",
    srcs = ["p4pd_mock.cc"],
    hdrs = [],
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
    ],
    copts = sdk_copts,
)
