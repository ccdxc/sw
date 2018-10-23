# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_test(
    name = "port_client",
    srcs = ["port_client.cc"],
    deps = [
        "//nic:gen_proto_includes",
        "//nic:halproto",
        "//nic:grpc",
        "//nic:grpc_includes",
        "//nic:google_includes",
        "//nic:libprotobuf",
        "//nic/delphi/proto/delphi:delphi_cc_proto",
    ],
    linkopts = [
        "-lpthread",
        "-pthread",
        "-lz",
        "-lm",
    ],
    copts = sdk_copts,
)

cc_test(
    name = "linkmgr_delphi_test",
    srcs = ["linkmgr_delphi_test.cc"],
    deps = [
        "//:gtest",
        "//nic:gen_proto_includes",
        "//nic:halproto",
        "//nic:grpc",
        "//nic:grpc_includes",
        "//nic:google_includes",
        "//nic:libprotobuf",
        "//nic/linkmgr:linkmgr_hdrs",
        "//nic/linkmgr:linkmgr_src",
        "//nic/proto/hal:hal_delphi_gen",
        "//nic/delphi/proto/delphi:delphi_cc_proto",
        "//nic/delphi/sdk",
        "//nic/linkmgr/test/utils:linkmgr_gtest_hdrs",
        "//nic/linkmgr/test/utils:linkmgr_test_utils",
        "@third_party//:libev",
        "@third_party//:libev_includes",
        "@sdk//obj:sdk_pal",
    ],
    linkopts = [
        "-lpthread",
        "-pthread",
        "-lz",
        "-lm",
        "-lJudy",
        "-ldl",
        "-lev",
        "-L/usr/local/lib",
    ],
    copts = sdk_copts,
)
