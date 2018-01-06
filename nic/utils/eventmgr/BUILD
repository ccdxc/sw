package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "eventmgr",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    copts = sdk_copts,
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
#        "//nic:sdk_ht",
#        "//nic:sdk_slab",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_list",
    ],
)
