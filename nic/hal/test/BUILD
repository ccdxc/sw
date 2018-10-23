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
	"//nic:gen_proto_includes",
        "//nic:google_includes",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/hal/core:core_includes",
        "//nic:spdlog",
        "@sdk//obj:sdk_thread",
    ],
    alwayslink = 1,
)

cc_library(
    name = "fte_mock",
    srcs = [
        "fte_mock.cc",
    ],
    deps = [
        "//nic/include:base_includes",
        "//nic/fte:fte_includes",
    ],
    alwayslink = 1,
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
        "//nic/delphi/proto/delphi:delphi_cc_proto",
        "//nic/proto/hal:hal_delphi_gen",
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
        "//nic/delphi/proto/delphi:delphi_cc_proto",
        "//nic/proto/hal:hal_delphi_gen",
        "//nic/delphi/sdk",
        "//nic:halproto",
        "//nic:grpc",
        "//nic:grpc_includes",
        "//nic:google_includes",
        "//nic:libprotobuf",
        "@sdk//obj:sdk_logger",
        "@sdk//obj:sdk_pal",
    ],
    linkopts = [
#"-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lm",
        "-ldl",
        "-rdynamic",
    ],
)

cc_binary(
    name = "nic_mgr_app",
    srcs = [
        "fake_nic_mgr.cc",
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
        "//nic/delphi/proto/delphi:delphi_cc_proto",
        "//nic/proto/hal:hal_delphi_gen",
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
