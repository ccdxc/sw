package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "eventmgr",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/list:list_includes",
        "//nic/utils/ht:ht",
        "//nic/utils/slab:slab",
    ],
)
