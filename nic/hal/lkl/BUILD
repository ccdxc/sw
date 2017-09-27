package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "lkl_api",
    srcs = [
        "lkl_api.cc",
    ],
    hdrs = ["lkl_api.hpp"],
    deps = [
            "//nic/hal:hal_includes",
            "//nic/include:base_includes",
            "//nic/third-party/spdlog",
            "//nic/utils/list:list_includes",
            "//nic/utils/thread:thread",
            "//nic:lkl_includes",
    ],
    includes = [
            "nic/third-party/lkl/export/include",
            ],
    copts = [
            "-DCONFIG_AUTO_LKL_POSIX_HOST",
            ],
)
