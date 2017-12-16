package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "twheel",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    #copts = sdk_copts,
    deps = [
        #"//nic:sdk_include",
        #"//nic:sdk_slab",
        #"@sdk//include:include",
        "@sdk//obj:sdk_slab",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
    ],
)

cc_test(
    name = "twheel_test",
    srcs = ["test/twheel_test.cc"],
    deps = [
        # Utils
        ":twheel",
        "//nic/utils/trace",
        #"//nic:sdk_logger",
        "@sdk//obj:sdk_logger",
        "//nic/hal/periodic",
        "//nic/hal/test:hal_mock",
        # External
        "//:gtest",
    ],
    #copts = sdk_copts,
)
