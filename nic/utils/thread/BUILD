package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "thread",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    copts = sdk_copts,
    deps = [
        #"//nic:sdk_ht",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_list",
        "//nic/hal:hal_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/hal/lib:hal_handle",
    ],
)
