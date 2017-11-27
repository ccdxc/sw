package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "plugins",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/fte",
        "//nic/third-party/spdlog",
    ],
    alwayslink = 1,
)
cc_library(
    name = "plugins_includes",
    srcs = [],
    hdrs = [
            "plugins.hpp"
            ],
)
