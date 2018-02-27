package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "hal_svc",
    srcs = [
    ] + glob(["*.cc"]),
    hdrs = [
    ] + glob(["*.hpp"]),
    copts = sdk_copts,
    deps = [
        "//nic:gen_proto_includes",
        "//nic/hal:hal_src",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic:grpc_includes",
    ],
)
