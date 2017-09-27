package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "hal_mock",
    srcs = [
        "hal_mock.cc",
    ],
    hdrs = [],
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/thread:thread",
    ],
    alwayslink = 1,
)
