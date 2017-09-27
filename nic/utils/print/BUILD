package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "print",
    srcs = glob(["*.cc"]),
    deps = [
        "//nic/hal:hal_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
    ],
)
