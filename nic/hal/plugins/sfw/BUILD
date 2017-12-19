package(default_visibility = ["//visibility:public"])
load("//nic/bazel:common_deps.bzl", "sdk_copts")

licenses(["notice"])  # MIT license

cc_library(
    name = "sfw",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic:gen_includes",
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/third-party/spdlog",
        "//nic/hal/plugins/app_redir:app_redir_includes",
    ],
    copts = sdk_copts,
    alwayslink = 1,
)

cc_library(
    name = "sfw_includes",
    hdrs = ["core.hpp"],
)    
