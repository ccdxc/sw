# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

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
    linkopts = [
        "-lJudy",
        "-lpthread",
        "-pthread",
        "-lz",
        "-rdynamic",
        "-lm",
        "-lrt",
    ],
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
            "//nic/utils/mtrack",
            "//nic/utils/agent_api:agent_api",

            # PD
            "@sdk//obj:sdk_catalog",
            "//nic:asic_libs",
            "//nic:libsknobs",
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
        "//nic/hal/core:core_includes",
        "//nic/include:base_includes",
        "//nic/hal/core:periodic_includes",
    ]
)

cc_library(
    name = "hal_src",
    srcs = [
        "hal.cc",
        "hal_state.cc",
        "hal_cfg.cc",
        "hal_obj.cc",
    ] + 
    glob(["src/stats/*.cc"]) +
    glob(["src/debug/*.cc"]) +
    glob(["src/internal/*.cc"]) +
    glob(["src/lif/*.cc"]) +
    glob(["src/dos/*.cc"]) +
    glob(["src/utils/*.cc"]),
    hdrs = [
        "hal.hpp",
    ] + 
    glob(["src/stats/*.hpp"]) +
    glob(["src/debug/*.hpp"]) +
    glob(["src/internal/*.hpp"]) +
    glob(["src/lif/*.hpp"]) +
    glob(["src/utils/*.hpp"]) +
    glob(["src/export/*.hpp"]),
    copts = sdk_copts,
    deps = [
        "//nic:asic_includes",
        "//nic:gen_proto_includes",
        "//nic:hal_gen_includes",
        "//nic/utils/trace",
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
        "//nic/hal/core:core",
        "//nic/hal/pd:pdcommon",
        "//nic/hal/core:periodic",
        "//nic/hal/core:core_includes",
        "//nic/hal/plugins/app_redir:app_redir_includes",
        "//nic/hal/plugins/network:network_includes",
        "//nic/hal/plugins/proxy:proxyplugin_includes",
        "//nic/hal/plugins/cfg/ipsec:ipsec_includes",
        "//nic/hal/plugins/cfg/gft:gft_includes",
        "//nic/hal/tls:tls_api",
        "//nic/include:base_includes",
        # TODO: PD-Cleanup: app_redir_ctx.hpp & rdma.cc
        "//nic/p4/include:p4_common_includes",
        "//nic:spdlog",
        "//nic/utils/block_list",
        "//nic/utils/bm_allocator",
        "//nic/utils/eventmgr",
        "//nic/utils/fsm",
        "//nic/utils/host_mem:host_mem_includes",
        "//nic/utils/print",
        "//nic/utils/bitmap:bitmap",
        "//nic:boost",
        "//nic/utils/nat:nat",
        "//nic:grpc_includes",
        "//nic/hal/plugins/cfg/nw:nw",
        "//nic/hal/plugins/cfg/aclqos:aclqos",
        "//nic/hal/plugins/sfw:sfw",
        "//nic/hal/plugins/cfg/nat:nat",
        "//nic/hal/plugins/cfg/gft:gft",
        "//nic/hal/plugins/cfg/ipsec:ipsec",
        "//nic/hal/plugins/cfg/mcast:mcast",
        "//nic/hal/plugins/cfg/telemetry:telemetry",
        "//nic/hal/plugins/cfg/dos:dos",
        "//nic/hal/plugins/cfg/l4lb:l4lb",
    ],
    alwayslink = 1,
)

cc_library(
    name = "hal_includes",
    srcs = [],
    hdrs = ["hal.hpp"] + 
        glob(["plugins/cfg/nw/*.hpp"]) +
        glob(["plugins/cfg/aclqos/*.hpp"]) +
        glob(["src/stats/*.hpp"]) +
        glob(["src/debug/*.hpp"]) +
        glob(["src/internal/*.hpp"]) +
        glob(["src/lif/*.hpp"]) +
        glob(["plugins/sfw/cfg/*.hpp"]) +
        glob(["src/ipsec/*.hpp"]) +
        glob(["src/utils/*.hpp"]) +
        glob(["src/gft/*.hpp"]) +
        glob(["plugins/cfg/telemetry/*.hpp"]) +
        glob(["plugins/cfg/nat/*.hpp"]) +
        glob(["plugins/cfg/mcast/*.hpp"]) +
        glob(["src/export/*.hpp"]),
    deps = [
        "//nic/hal/core:core_includes",
        "//nic/hal:hal_hdrs",
        "//nic/p4/include:p4_common_includes",
        "//nic:asic_includes",
        "//nic:gen_proto_includes",
        "//nic:hal_gen_includes",
        "//nic/hal/plugins/cfg/aclqos:aclqos_includes",
        "//nic/hal/plugins/cfg/nw:nw_includes",
        "//nic/hal/plugins/cfg/telemetry:telemetry_includes",
        "//nic/hal/plugins/cfg/mcast:mcast_includes",
        "//nic/hal/plugins/sfw:firewall_includes",
        "//nic/hal/plugins/cfg/nat:nat_includes",
        "//nic/hal/plugins/cfg/ipsec:ipsec_includes",
        "//nic/hal/plugins/cfg/gft:gft_includes",
    ]
)

cc_library(
    name = "hal_utils_includes",
    srcs = [],
    hdrs = ["src/utils/utils.hpp"],
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
            "//nic/utils/trace",
            "//nic/utils/mtrack",
            "@gperftools//:gperftools",

            # PD
            "@sdk//obj:sdk_catalog",
            "//nic:asic_libs",
        ],
)
