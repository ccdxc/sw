package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pt",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/hal:hal_includes",
        "//nic/hal/periodic",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/print",
        "//nic/utils/slab",
        "//nic/utils/thread",
    ],
)

cc_test(
    name = "pt_test",
    srcs = ["test/pt_test.cc"],
    deps = [
        ":pt",
        "//nic/utils/trace",
        "//nic/hal/test:hal_mock",
        # External
        "//:gtest",
    ],
)
