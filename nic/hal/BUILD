package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")

cc_binary(
    name = "hal",
    srcs = [
        "main.cc",
    ],
    copts = sdk_copts,
    linkopts = gl_linkopts_list,
    deps = [
            ":hal_src",
            # PI
            "//nic/fte",
            "//nic:hal_svc_gen",
            "//nic/hal/svc:hal_svc",
            "//nic/utils/print",
            "//nic/hal/plugins",
            "//nic/hal/plugins/proxy:proxyplugin",
            "//nic/hal/lkl:lkl_api",
            "//nic:lkl",
            "//nic:halproto",
            "//nic:grpc_includes",
            "//nic:libprotobuf",
            "//nic:grpc",
            "//nic/hal/third-party/isc-dhcp:isc-dhcp",  # TODO(move to eplearn plugin)
            "//nic/utils/trace",
            "//nic/utils/mtrack",

            # PD
            "@sdk//obj:sdk_catalog",
            "//nic:asic_libs",
        ],
)

config_setting(
   name = "gft",
   values = { "define": "p4pipeline=gft" }
)

cc_library(
    name = "hal_hdrs",
    hdrs = [
        "hal.hpp",
    ] + glob(["src/*.hpp"]),
    deps = [
        "//nic/hal/lib:hal_lib_includes",
        "//nic/include:base_includes",
        "//nic/hal/periodic:hal_periodic_includes",
    ]
)

cc_library(
    name = "hal_src",
    srcs = [
        "hal.cc",
        "hal_state.cc",
        "hal_cfg.cc",
    ] + glob(["src/*.cc"]),
    hdrs = [
        "hal.hpp",
    ] + glob(["src/*.hpp"]) +
    glob(["src/export/*.hpp"]),
    copts = sdk_copts,
    deps = [
        "//nic:asic_includes",
        "//nic:gen_proto_includes",
        "//nic:hal_gen_includes",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_indexer",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_logger",
        "@sdk//obj:sdk_thread",
        "@sdk//obj:sdk_utils",
        "@sdk//obj:sdk_shmmgr",
        "//nic/asm/cpu-p4plus/include:cpu_p4plus_includes",
        "//nic/fte:fte_includes",
        "//nic/hal:hal_includes",
        "//nic/hal/lib:hal_lib",
        "//nic/hal/pd:pdcommon",
        "//nic/hal/periodic",
        "//nic/hal/plugins:plugins_includes",
        "//nic/hal/plugins/app_redir:app_redir_includes",
        "//nic/hal/plugins/network:network_includes",
        "//nic/hal/plugins/proxy:proxyplugin_includes",
        "//nic/hal/tls:tls_api",
        "//nic/include:base_includes",
        # TODO: PD-Cleanup: app_redir_ctx.hpp & rdma.cc
        "//nic/p4/include:p4_common_includes",
        "//nic/p4/iris/include:p4_iris_includes",   # TODO: this should go away ACL & app-redir are fixed
        "//nic:spdlog",
        "//nic/utils/block_list",
        "//nic/utils/bm_allocator",
        "//nic/utils/eventmgr",
        "//nic/utils/fsm",
        "//nic/utils/host_mem:host_mem_includes",
        "//nic/utils/print",
        "//nic/utils/bitmap:bitmap",
        "//nic:boost",
        "//nic:grpc_includes",
    ],
    alwayslink = 1,
)

cc_library(
    name = "hal_includes",
    srcs = [],
    hdrs = ["hal.hpp"] + glob([
        "src/*.hpp",
    ]),
)

cc_library(
    name = "hal_utils_includes",
    srcs = [],
    hdrs = ["src/utils.hpp"],
)

cc_binary(
    name = "hal_perf",
    srcs = [
        "main.cc",
    ],
    copts = sdk_copts,
    linkopts = gl_linkopts_list,
    deps = [
            ":hal_src",
            # PI
            "//nic/fte",
            "//nic:hal_svc_gen",
            "//nic/hal/svc:hal_svc",
            "//nic/utils/print",
            "//nic/hal/plugins",
            "//nic/hal/plugins/proxy:proxyplugin",
            "//nic/hal/lkl:lkl_api",
            "//nic:lkl",
            "//nic:halproto",
            "//nic:grpc_includes",
            "//nic:libprotobuf",
            "//nic:grpc",
            "//nic/hal/third-party/isc-dhcp:isc-dhcp",  # TODO(move to eplearn plugin)
            "//nic/utils/trace",
            "//nic/utils/mtrack",
            "@gperftools//:gperftools",

            # PD
            "@sdk//obj:sdk_catalog",
            "//nic:asic_libs",
        ],
)
