package(default_visibility = ["//visibility:public"])
load("//nic/bazel:common_deps.bzl", "sdk_copts")

licenses(["notice"])  # MIT license

cc_library(
    name = "classic",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic:gen_includes",
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/third-party/spdlog",
        "//nic:gen_proto_includes",
    ],
    copts = sdk_copts,
    alwayslink = 1,
)
