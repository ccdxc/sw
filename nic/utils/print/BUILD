package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "print",
    srcs = glob(["*.cc"]),
    deps = [
        "//nic/hal:hal_includes",
        "//nic/include:base_includes",
        "//nic:spdlog",
        ],
    copts = sdk_copts,
)
