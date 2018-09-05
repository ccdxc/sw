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
    ],
    linkopts = [
        "-lpthread",
        "-pthread",
        "-lz",
        "-lm",
    ],
    copts = sdk_copts,
)
