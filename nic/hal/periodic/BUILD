package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "periodic",
    srcs = [
        "periodic.cc",
    ],
    hdrs = ["periodic.hpp"],
    #copts = sdk_copts,
    deps = [
        "//nic/hal:hal_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        #"//nic:sdk_include",
        #"@sdk:sdk_headers",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_ht",
        "//nic/utils/thread:thread",
        "//nic/utils/twheel:twheel",
    ],
)
