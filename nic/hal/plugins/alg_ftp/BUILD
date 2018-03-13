package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "alg_ftp",
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
    name = "alg_ftp_includes",
    hdrs = ["core.hpp"],
)
