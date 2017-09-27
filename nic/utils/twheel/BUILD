package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "twheel",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/slab",
    ],
)

cc_test(
    name = "twheel_test",
    srcs = ["test/twheel_test.cc"],
    deps = [
        # Utils
        ":twheel",
        "//nic/utils/trace",
        "//nic/hal/periodic",
        "//nic/hal/test:hal_mock",
        # External
        "//:gtest",
    ],
)
