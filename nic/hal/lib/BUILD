package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "hal_lib_includes",
    hdrs = [
        "hal_handle.hpp",
    ],
    deps = [
        "@sdk//obj:sdk_list",
    ]
)

cc_library(
    name = "hal_lib",
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
        "//nic:spdlog",
        "//nic:boost",
        "//nic/hal/periodic:periodic",
    ],
    copts = sdk_copts,
)
