package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "sfw",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic:gen_includes",
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
