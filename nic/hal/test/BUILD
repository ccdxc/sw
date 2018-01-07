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
        "//nic/hal/lib:hal_handle_includes",
        "//nic/third-party/spdlog",
        "@sdk//obj:sdk_thread",
    ],
    alwayslink = 1,
    #copts = sdk_copts,
)

cc_binary(
    name = "hal_test",
    srcs = [
        "hal_test.cc",
    ],
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "@sdk//obj:sdk_thread",
        "//nic:gen_proto_includes",
        "//nic:halproto",
        "//nic:grpc",
        "@sdk//obj:sdk_logger",
    ],
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lprotobuf",
    ],
)
