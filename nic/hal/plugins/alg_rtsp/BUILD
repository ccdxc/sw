package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "alg_rtsp",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic/fte:fte",
        "//nic:gen_proto_includes",
        "//nic:grpc",
        "//nic:halproto",
        "//nic/hal:hal_src",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/hal/plugins/sfw:sfw_includes",
        "//nic/hal/plugins/alg_utils:alg_utils",
    ],
    alwayslink = 1,
    copts = sdk_copts,
)

cc_test(
    name = "alg_rtsp_test",
    srcs = glob(["test/*.cc"]),
    copts = sdk_copts,
    linkopts =  ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    tags = ["gtests"],
    deps =  ["//nic/fte:fte_base_test",
             ":alg_rtsp",
            ] + gl_deps_list + test_deps_list, 
)
