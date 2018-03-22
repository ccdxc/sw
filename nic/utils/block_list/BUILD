package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "block_list",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic:spdlog",
        "@sdk//obj:sdk_list",
        "@sdk//obj:sdk_shmmgr",
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
        "//nic/utils/mtrack",
        "@sdk//obj:sdk_logger",
        "//nic/hal/test:hal_mock",
        # External
        "//:gtest",
    ],
    linkopts = [
        "-lrt",
    ],
    copts = sdk_copts,
)
