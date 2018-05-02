package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "agent_api",
    srcs = glob(["*.cc","*.c"]),
    hdrs = glob(["*.hpp","*.h"]),
    copts = sdk_copts,
    deps = [
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic/hal:hal_src",
    ],
)

cc_test(
    name = "agent_api_test",
    srcs = ["test/ipc_test.cc"],
    deps = [":agent_api"] + gl_deps_list + test_deps_list,
    linkopts = [
        "-lrt",
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lm",
    ],
    copts = [
        "-Wno-write-strings",
        ] + sdk_copts,
)
