package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")

cc_library(
    name = "tcam",
    srcs = [
        "tcam.cc",
        "tcam.hpp",
        "tcam_entry.cc",
        "tcam_entry.hpp",
    ],
    hdrs = [
        "tcam.hpp",
        "tcam_entry.hpp",
    ],
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        "//nic/hal/pd/utils/acl_tcam:acl_tcam_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/indexer",
        #"//nic/utils/thread",
    ],
)

cc_library(
    name = "tcam_includes",
    srcs = [],
    hdrs = [
        "tcam.hpp",
        "tcam_entry.hpp",
    ],
)

cc_test(
    name = "tcam_test",
    srcs = ["test/tcam_test.cc"],
    deps = gl_deps_list,
    linkopts = gl_linkopts_list
)
