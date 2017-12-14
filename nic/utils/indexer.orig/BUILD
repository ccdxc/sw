package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "indexer",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
    ],
)

cc_test(
    name = "indexer_test",
    srcs = ["test/indexer_test.cc"],
    deps = [
        # Utils
        ":indexer",
        "//nic/utils/trace",
        # External
        "//:gtest",
    ],
)
