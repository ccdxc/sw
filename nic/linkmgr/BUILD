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
            ] + glob(["*.cc"]),
    deps = [
        "//nic/hal:hal_includes",
        "//nic:asic_includes",
        "//nic/include:base_includes",
        "//nic:gen_proto_includes",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_indexer",
        "@sdk//obj:sdk_logger",
        "@sdk//obj:sdk_pal",
        "@sdk//obj:sdk_thread",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/third-party/spdlog",
        "@sdk//obj:sdk_twheel",
        "//nic/utils/mtrack:mtrack",
        "@sdk//obj:sdk_catalog",
        "//nic/utils/block_list:block_list",
        "//nic/linkmgr:linkmgr_includes",
        "//nic/hal/lib:hal_handle_includes",
        "//nic/hal/lib:hal_handle",
        "//nic/hal/periodic",
        "//nic:halproto",
        "//nic:grpc",
        "//nic/utils/trace:trace",
        "//nic/model_sim:model_client",
    ],
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lprotobuf",
        "-rdynamic",
        "-ldl",
        "-lm",
    ],
    copts = sdk_copts,
)
