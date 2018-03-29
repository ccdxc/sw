package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "sfw",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic:spdlog",
        "//nic/hal/plugins/app_redir:app_redir_includes",
        "//nic/hal/plugins/alg_utils:alg_utils_includes",
    ],
    alwayslink = 1,
    copts = sdk_copts,
)

cc_library(
    name = "sfw_includes",
    hdrs = ["core.hpp"],
)    

cc_test(
    name = "sfw_test",
    srcs = glob(["test/*.cc", "test/*.hpp"]),
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    deps = ["//nic/fte:fte_base_test"] + gl_deps_list + test_deps_list,
    copts = sdk_copts,
    tags = ["gtests"],
)
