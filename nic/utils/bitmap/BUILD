package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "bitmap",
    srcs = ["bitmap.cc", "bitmap.hpp"],
    hdrs = ["bitmap.hpp"],
    deps = ["//nic/include:base_includes", "//nic/third-party/spdlog:spdlog", "//nic/hal:hal_includes"],
    copts = sdk_copts,
)

cc_test(
     name = "bitmap_test",
     srcs = ["test/bitmap_test.cc"],
     deps = [
        ":bitmap",
        "//nic/hal/test:hal_mock",
        "//nic/utils/trace:trace",
        "//nic/hal/periodic",
        "//nic:sdk_logger",

        # External
        "//:gtest",
     ],
    copts = sdk_copts,
)
