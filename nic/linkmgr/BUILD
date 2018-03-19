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
        "@sdk//obj:sdk_shmmgr",
        "@sdk//obj:sdk_catalog",
        "@sdk//obj:sdk_utils",
        "//nic:spdlog",
        "//nic/utils/mtrack:mtrack",
        "//nic/utils/block_list:block_list",
        "//nic/linkmgr:linkmgr_includes",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/hal/lib:hal_lib",
        "//nic:halproto",
        "//nic:grpc",
        "//nic:grpc_includes",
        "//nic:libprotobuf",
        "//nic/utils/trace:trace",
        "@sdk//obj:sdk_linkmgr",
        "//nic:google_includes",
    ],
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-rdynamic",
        "-ldl",
        "-lm",
        "-lrt",
    ],
    copts = sdk_copts,
)
