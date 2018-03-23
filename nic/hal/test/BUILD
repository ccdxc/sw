package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "hal_mock",
    srcs = [
        "hal_mock.cc",
    ],
    deps = [
        "//nic/include:base_includes",
        "//nic/hal/lib:hal_lib_includes",
        "//nic:spdlog",
        "@sdk//obj:sdk_thread",
    ],
    alwayslink = 1,
    #copts = sdk_copts,
)

cc_binary(
    name = "hal_scale_test",
    srcs = [
        "hal_scale_test.cc",
    ],
    deps = [
        "//nic/include:base_includes",
        "//nic:spdlog",
        "@sdk//obj:sdk_thread",
        "//nic:gen_proto_includes",
        "//nic:halproto",
        "//nic:grpc",
        "//nic:grpc_includes",
        "//nic:google_includes",
        "//nic:libprotobuf",
        "@sdk//obj:sdk_logger",
    ],
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lm",
    ],
)

cc_binary(
    name = "hal_test",
    srcs = [
        "hal_test.cc",
    ],
    deps = [
        "//nic/include:base_includes",
        "//nic:spdlog",
        "@sdk//obj:sdk_thread",
        "//nic:gen_proto_includes",
        "//nic:halproto",
        "//nic:grpc",
        "//nic:grpc_includes",
        "//nic:google_includes",
        "//nic:libprotobuf",
        "@sdk//obj:sdk_logger",
    ],
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lm",
    ],
)
