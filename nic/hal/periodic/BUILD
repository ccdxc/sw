package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "periodic",
    srcs = [
        "periodic.cc",
    ],
    hdrs = ["periodic.hpp"],
    deps = [
        "//nic/hal:hal_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/thread",
        "//nic/utils/twheel",
    ],
)
