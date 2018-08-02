package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "haltestutils",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic:boost",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_indexer",
        "@sdk//obj:sdk_slab",
        "//nic/fte:fte_includes",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/utils/mtrack:mtrack",
        "@sdk//obj:sdk_list",
        "//nic/hal/lib:hal_lib_includes",
        "//:gtest",
        "//nic/hal/core:periodic",
        "//nic/hal/pd:pdcommon",
    ],
    copts = sdk_copts
)
