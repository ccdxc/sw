package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "network",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic:gen_includes",
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic/hal/plugins/eplearn:eplearn_includes",
        "//nic/hal/plugins/eplearn:eplearn",
        "//nic:gen_proto_includes",
     ],
    alwayslink = 1,
    copts = sdk_copts,
)

cc_library(
    name = "network_includes",
    hdrs = ["net_plugin.hpp"],
)

