package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "hal_handle_includes",
    hdrs = [
        "hal_handle.hpp",
    ],
    deps = [
        "@sdk//obj:sdk_list",
    ]
)

cc_library(
    name = "hal_handle",
    srcs = [
        "hal_handle.cc",
    ],
    hdrs = [
        "hal_handle.hpp",
    ],
    deps = [
        "@sdk//obj:sdk_list",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_ht",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
    ],
    copts = sdk_copts,
)
