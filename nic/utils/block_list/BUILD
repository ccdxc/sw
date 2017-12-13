package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "block_list",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/list:list_includes",
        "//nic/utils/iterator:iterator",
    ],
    copts = sdk_copts,
)

cc_test(
    name = "block_list_test",
    srcs = ["test/block_list_test.cc"],
    deps = [
        # Utils
        ":block_list",
        "//nic/utils/trace",
        "//nic:sdk_logger",
        "//nic/hal/periodic",
        "//nic/hal/test:hal_mock",
        # External
        "//:gtest",
    ],
    copts = sdk_copts,
)
