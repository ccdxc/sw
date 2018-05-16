package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "nat",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic:spdlog",
    ],
    alwayslink = 1,
    copts = sdk_copts,
)

cc_test(
    name = "nat_test",
    srcs = glob(["test/*.cc"]),
    copts = sdk_copts,
    linkopts =  ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    tags = ["gtests"],
    deps =  ["//nic/fte:fte_base_test"] + [":nat"] + gl_deps_list + test_deps_list, 
)