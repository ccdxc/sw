package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")

cc_library(
    name = "fte",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/hal:hal_src",
        "//nic/hal/svc:hal_svc",
        "//nic:gen_includes",
        "//nic:gen_proto_includes",
        "//nic/p4/nw/include:p4_nw_includes",
        "//nic/asm/cpu-p4plus/include:cpu_p4plus_includes",
        "//nic/third-party/spdlog:spdlog",
        "//nic:halproto",
        "//nic:grpc",
    ],
    alwayslink = 1,
)

cc_library(
    name = "fte_includes",
    hdrs = ["fte.hpp", "fte_ctx.hpp"],
)

cc_test(
    name = "fte_test",
    srcs = glob(["test/*.cc"]),
    deps = gl_deps_list,
    linkopts = gl_linkopts_list
)
