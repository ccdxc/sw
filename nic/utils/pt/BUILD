package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "pt",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/hal:hal_includes",
        "//nic/hal/periodic",
        "//nic/include:base_includes",
        "//nic/hal/lib:hal_handle_includes",
        "//nic:spdlog",
        "//nic/utils/print",
        "@sdk//obj:sdk_slab",
    ],
    copts = sdk_copts,
)

cc_test(
    name = "pt_test",
    srcs = ["test/pt_test.cc"],
    deps = [
        ":pt",
        "//nic/utils/trace",
        "@sdk//obj:sdk_logger",
        "//nic/hal/test:hal_mock",
        # External
        "//:gtest",
    ],
    copts = sdk_copts,
)
