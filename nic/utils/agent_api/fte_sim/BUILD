package(default_visibility = ["//visibility:public"])

load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_library(
    name = "fte_sim_lib",
    srcs = [
        "fwlogsim_svc.cc",
    ],
    hdrs = glob(["*.hpp","*.h"]),
    copts = sdk_copts,
    deps = [
        "//nic:gen_proto_includes",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic:google_includes",
        "@sdk//obj:sdk_logger",
    ],
)

cc_binary(
    name = "fte_sim",
    srcs = [
        "main.cc",
    ],
    deps = [
        "fte_sim_lib",
        "//nic:grpc",
        "//nic/utils/agent_api:agent_api",
        "//nic/utils/mtrack:mtrack",
        "//nic:libprotobuf",
        "//nic:halproto",
    ],
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lm",
        "-lrt",
    ],
)
