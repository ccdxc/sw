package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "hash",
    srcs = [
        "hash.cc",
        "hash.hpp",
        "hash_entry.cc",
        "hash_entry.hpp",
    ],
    hdrs = [
        "hash.hpp",
        "hash_entry.hpp",
    ],
    copts = sdk_copts,
    deps = [
        "@sdk//obj:sdk_indexer",
        "//nic/hal/pd:hal_pd_includes",
        #"//nic/hal/pd/utils/tcam",
        "@sdk//obj:sdk_tcam",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
    ],
)

cc_library(
    name = "hash_includes",
    srcs = [],
    hdrs = ["hash.hpp"],
)

cc_test(
    name = "hash_test",
    srcs = ["test/hash_test.cc"],
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lprotobuf",
    ],
    deps = gl_deps_list + [
        "//nic/hal/pd/utils/p4pd_mock:p4pd_mock",
    ],
    copts = sdk_copts,
)
