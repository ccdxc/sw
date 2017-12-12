package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "thread",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/hal:hal_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/ht",
        "//nic/utils/list:list_includes",
        "//nic/hal/lib:hal_handle",
    ],
)
