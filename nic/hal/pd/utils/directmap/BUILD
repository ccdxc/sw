package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "directmap",
    srcs = [
        "directmap.cc",
        "directmap.hpp",
        "directmap_entry.cc",
        "directmap_entry.hpp",
    ],
    hdrs = ["directmap.hpp"],
    copts = sdk_copts,
    deps = [
        #"//nic:sdk_ht",
        #"//nic:sdk_indexer",
        #"//nic:sdk_slab",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_indexer",
        "@sdk//obj:sdk_slab",
        "//nic/hal/pd:hal_pd_includes",
        "//nic/hal/pd/iris:iris_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/thread",
        "//nic/hal/pd/capri:capri_includes",
    ],
)

cc_library(
    name = "directmap_includes",
    srcs = [],
    hdrs = [
        "directmap.hpp",
        "directmap_entry.hpp",
    ],
)

cc_test(
    name = "directmap_test",
    srcs = ["test/directmap_test.cc"],
    linkopts = gl_linkopts_list,
    deps = gl_deps_list,
    copts = sdk_copts,
)
