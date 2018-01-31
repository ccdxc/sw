# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "linkmgr_includes",
    hdrs = [
    ] + glob(["*.h", "*.hpp"])
)

cc_binary(
    name = "linkmgr",
    srcs = [
            "linkmgr.cc",
            "linkmgr_src.cc",
            "linkmgr_svc.cc",
            "linkmgr_state.cc",
            "utils.cc",
            ],
    deps = [
        "//nic:gen_proto_includes",
        "@sdk//obj:sdk_logger",
        "@sdk//obj:sdk_pal",
        "//nic:spdlog",
        "//nic/utils/mtrack:mtrack",
        "@sdk//obj:sdk_catalog",
        "//nic/utils/block_list:block_list",
        "//nic/linkmgr:linkmgr_includes",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/hal/lib:hal_lib",
        "//nic:halproto",
        "//nic:grpc",
        "//nic:libprotobuf",
        "//nic/utils/trace:trace",
        "@sdk//linkmgr:linkmgr",
    ],
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-rdynamic",
        "-ldl",
        "-lm",
    ],
    copts = sdk_copts,
)
