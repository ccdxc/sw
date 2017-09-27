package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "thread",
    srcs = [
        "thread.cc",
        "thread.hpp",
    ],
    hdrs = ["thread.hpp"],
    deps = [
        "//nic/hal:hal_includes",
        "//nic/utils/ht:ht",
        "//nic/utils/list:list_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
    ],
)
