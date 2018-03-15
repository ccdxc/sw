package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "fte",
    srcs = glob(["*.cc", "acl/*.cc"]),
    hdrs = glob(["*.hpp", "acl/*.hpp"]),
    copts = sdk_copts,
    deps = [
        "//nic:gen_proto_includes",
        "//nic:grpc",
        "//nic:halproto",
        "//nic/hal:hal_src",
        "//nic:hal_svc_gen",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic/hal/plugins/app_redir:app_redir_includes",
    ],
    alwayslink = 1,
)

cc_library(
    name = "fte_includes",
    hdrs = [
        "fte.hpp",
        "fte_ctx.hpp",
        "acl/acl.hpp",
        "acl/ref.hpp"
    ]
)

cc_test(
    name = "fte_test",
    srcs = glob(["test/*.cc"]),
    linkopts = gl_linkopts_list,
    deps = gl_deps_list + test_deps_list,
    copts = sdk_copts,
)
