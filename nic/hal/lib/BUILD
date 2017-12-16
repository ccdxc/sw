package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "hal_handle_includes",
    hdrs = [
        "hal_handle.hpp",
    ],
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
        "//nic/utils/list:list_includes",
        #"//nic:sdk_include",
        #"//nic:sdk_ht",
        #"//nic:sdk_slab",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_ht",
        "//nic/third-party/spdlog",
    ],
    copts = sdk_copts,
)
