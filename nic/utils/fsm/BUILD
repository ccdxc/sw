package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "fsm",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/list:list_includes",
    ],
)

cc_test(
    name = "fsm_test",
    srcs = ["test/fsm_test.cc"],
    deps = [
        # Utils
        ":fsm",
        "//nic/utils/trace",
        # External
        "//:gtest",
    ],
)
