package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

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
        "//nic/utils/indexer:indexer",
        "//nic/model_sim/include:modelsim_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/thread",
        "//nic/utils/twheel",
        "//nic/utils/mtrack:mtrack",
        "//nic/utils/catalog",
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
    ],
)
