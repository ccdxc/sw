package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "list_includes",
    hdrs = ["list.hpp"],
    deps = [
        "//nic/include:base_includes",
    ],
)

cc_test(
    name = "list_test",
    srcs = ["test/list_test.cc"],
    deps = [
        ":list_includes",
        "//nic/utils/trace",
        "//nic:sdk_logger",
        # External
        "//:gtest",
    ],
    copts = sdk_copts,
)
