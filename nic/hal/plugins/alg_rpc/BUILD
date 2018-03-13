package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "alg_rpc",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/hal/plugins/sfw:sfw_includes",
        "//nic/hal/plugins/alg_utils:alg_utils_includes",
        "//nic:libtirpc",
    ],
    alwayslink = 1,
    copts = sdk_copts,
)

cc_library(
    name = "alg_rpc_includes",
    hdrs = ["core.hpp", "msrpc_proto_def.hpp", "alg_msrpc.hpp"],
)

cc_test(
    name = "alg_rpc_test",
    srcs = glob(["test/*.cc"]),
    deps = [
           "@sdk//obj:sdk_thread",
           "//nic:gen_proto_includes",
           "//nic:grpc_includes",
           "//nic:google_includes",
           "@sdk//obj:sdk_logger",
           "//nic/hal/plugins/alg_utils:alg_utils",
           "//nic/hal/plugins/alg_rpc:alg_rpc",
           ] + gl_deps_list + test_deps_list,
    linkopts = gl_linkopts_list,
    copts = sdk_copts
)
