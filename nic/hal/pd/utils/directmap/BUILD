package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")

cc_library(
    name = "directmap",
    srcs = [
        "directmap.cc",
        "directmap.hpp",
        "directmap_entry.cc",
        "directmap_entry.hpp",
    ],
    hdrs = ["directmap.hpp"],
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        #"//nic/include:base_includes",
        #"//nic/third-party/spdlog",
        "//nic/utils/indexer",
        #"//nic/utils/thread",
        "//nic/hal/pd/iris:iris_includes",
        "//nic/utils/ht:ht",
        "//nic/utils/slab:slab",
        #"//nic/utils/bm_allocator:bm_allocator",
    ],
)

cc_library(
    name = "directmap_includes",
    srcs = [],
    hdrs = ["directmap.hpp",
            "directmap_entry.hpp"],
)

cc_test(
    name = "directmap_test",
    srcs = ["test/directmap_test.cc"],
    deps = gl_deps_list,
    linkopts = gl_linkopts_list,
)
