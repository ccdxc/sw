package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "twheel",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "@sdk//obj:sdk_slab",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
    ],
)

cc_test(
    name = "twheel_test",
    srcs = ["test/twheel_test.cc"],
    deps = [
        ":twheel",
        "//nic/utils/trace",
        "@sdk//obj:sdk_logger",
        "//nic/hal/periodic",
        "//nic/hal/test:hal_mock",
        "//:gtest",
    ],
)
