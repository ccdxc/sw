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
        "//nic/hal:hal_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/list:list_includes",
        "//nic/hal/lib:hal_handle",
    ],
)
