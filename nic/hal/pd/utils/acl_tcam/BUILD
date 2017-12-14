package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "acl_tcam",
    srcs = [
        "acl_tcam.cc",
        "acl_tcam.hpp",
    ],
    hdrs = ["acl_tcam.hpp"],
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        "//nic/hal/pd/utils/tcam:tcam_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/bitmap",
        "//nic/utils/thread",
    ],
    copts = sdk_copts,
)

cc_library(
    name = "acl_tcam_includes",
    srcs = [],
    hdrs = ["acl_tcam.hpp"],
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/bitmap",
        "//nic/utils/thread",
    ],
)

cc_test(
    name = "acl_tcam_test",
    srcs = ["test/acl_tcam_test.cc"],
    deps = gl_deps_list,
    linkopts = [
        "-lzmq",
        "-lprotobuf"
     ],
    copts = sdk_copts,
)
