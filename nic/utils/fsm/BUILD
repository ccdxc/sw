package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")


cc_library(
    name = "fsm",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/list:list_includes",
    ],
    alwayslink = 1,
)

cc_test(
    name = "fsm_test",
    srcs = ["test/fsm_test.cc"],
    deps = [
        # Utils
        ":fsm",
    ] + gl_deps_list,
    linkopts = gl_linkopts_list,
)
