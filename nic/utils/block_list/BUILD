package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "block_list",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/list:list_includes",
    ],
)

cc_test(
    name = "block_list_test",
    srcs = ["test/block_list_test.cc"],
    deps = [
        # Utils
        ":block_list",
        "//nic/utils/trace",
        "//nic/hal/periodic",
        "//nic/hal/test:hal_mock",
        # External
        "//:gtest",
    ],
)
