package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "catalog",

    srcs = [
    ] + glob(["*.cc"]),

    hdrs = [
    ] + glob(["*.hpp"]),

    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog:spdlog",
        "//nic/hal:hal_includes",
        "//nic:gen_proto_includes"
    ],
    copts = sdk_copts,
)
