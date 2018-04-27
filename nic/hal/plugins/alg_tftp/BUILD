package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "alg_tftp",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/hal/plugins/sfw:sfw_includes",
        "//nic/hal/plugins/alg_utils:alg_utils_includes",
    ],
    alwayslink = 1,
    copts = sdk_copts,
)

cc_library(
    name = "alg_tftp_includes",
    hdrs = ["core.hpp"],
)

cc_test(
    name = "alg_tftp_test",
    srcs = glob(["test/*.cc"]),
    deps = [
           "//nic/hal/plugins/alg_utils:alg_utils",
           ":alg_tftp",
           "//nic/fte:fte_base_test",
           ] + gl_deps_list + test_deps_list,
    linkopts =  ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    tags = ["gtests"],
    copts = sdk_copts,
)
