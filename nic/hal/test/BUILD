package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "hal_mock",
    srcs = [
        "hal_mock.cc",
    ],
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/thread:thread",
    ],
    alwayslink = 1
)

cc_binary(
    name = "hal_test",
    srcs = [
        "hal_test.cc",
    ],
    deps = [
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/thread:thread",
        "//nic:gen_proto_includes",
        "//nic:halproto",
        "//nic:grpc",
    ],
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lprotobuf",
    ],
)
