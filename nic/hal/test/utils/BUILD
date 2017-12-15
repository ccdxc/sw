package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "haltestutils",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/catalog:catalog",
        "//nic:sdk_slab",
        "//nic:sdk_indexer",
        "//nic:sdk_ht",
        "//nic/utils/thread:thread",
        "//nic/hal/pd/iris:iris_includes",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/utils/mtrack:mtrack",
        "//:gtest",
        "//nic/hal/pd/capri:capri_includes",
    ],
    copts = sdk_copts
)
