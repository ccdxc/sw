package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")

cc_library(
    name = "alg",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic:gen_includes",
        "//nic/fte",
        "//nic/hal/plugins/sfw:sfw_includes",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/third-party/spdlog",
        "//nic:libtirpc",
    ],
    alwayslink = 1,
)

cc_library(
    name = "alg_includes",
    hdrs = ["alg_tftp.hpp", "alg_rpc.hpp"],
)

cc_test(
    name = "alg_test",
    srcs = glob(["test/*.cc"]),
    deps = gl_deps_list + [":alg"],
    linkopts = gl_linkopts_list,
)
