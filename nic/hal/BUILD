package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_binary(
    name = "hal",
    srcs = [
        "main.cc",
    ],
    copts = sdk_copts,
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lprotobuf",
        "-rdynamic",
        "-lm",
    ],
    deps = [
        ":hal_src",
        # PI
        "//nic/fte",
        "//nic/hal/periodic",
        "//nic/hal/svc:hal_svc",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem",
        "//nic/hal/plugins",
        "//nic/hal/plugins/proxy:proxyplugin",
        "//nic/hal/lkl:lkl_api",
        "//nic:lkl",
        "//nic:halproto",
        "//nic:grpc",
        "//nic/hal/lib:hal_handle",
        "//nic/third-party/isc-dhcp",  # TODO(move to eplearn plugin)

        # PD
        "//nic/hal/pd/common:pdcommon",
        "//nic/hal/pd/utils/hash",
        "@sdk//obj:sdk_tcam",
        "@sdk//obj:sdk_directmap",
        "//nic/hal/pd/utils/flow",
        "//nic/hal/pd/utils/met",
        "//nic/hal/pd/utils/acl_tcam",
        "//nic/hal/pd/iris/p4pd:p4pdapi",
        "//nic/hal/pd/iris",
        "//nic:capricsr_int",
        "//nic/hal/pd/capri",
        "//nic/hal/pd/control",
        "//nic:p4pd",
        "//nic:gft_p4pd",
        "//nic:p4pluspd_txdma",
        "//nic:p4pluspd_rxdma",
        "//nic:asic_libs",
    ],
)

cc_library(
    name = "hal_hdrs",
    hdrs = [
        "hal.hpp",
    ] + glob(["src/*.hpp"]),
    deps = [
        "//nic/hal/lib:hal_handle_includes",
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
    ] + glob(["src/*.hpp"]),
    copts = sdk_copts,
    deps = [
        "//nic:asic_includes",
        "//nic:gen_includes",
        "//nic:gen_proto_includes",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_indexer",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_logger",
        "@sdk//obj:sdk_thread",
        "//nic/asm/cpu-p4plus/include:cpu_p4plus_includes",
        "//nic/fte:fte_includes",
        "//nic/hal:hal_includes",
        "//nic/hal/lib:hal_handle_includes",
        "//nic/hal/pd/capri:capri_includes",
        "//nic/hal/pd/common:pdcommon",
        "//nic/hal/pd/iris:iris_includes",
        "//nic/hal/periodic",
        "//nic/hal/plugins:plugins_includes",
        "//nic/hal/plugins/app_redir:app_redir_includes",
        "//nic/hal/plugins/network:network_includes",
        "//nic/hal/plugins/proxy:proxyplugin_includes",
        "//nic/hal/tls:tls_api",
        "//nic/include:base_includes",
        "//nic/p4/include:p4_common_includes",
        "//nic/p4/nw/include:p4_nw_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/block_list",
        "//nic/utils/bm_allocator",
        "//nic/utils/eventmgr",
        "//nic/utils/fsm",
        "//nic/utils/host_mem:host_mem_includes",
        "//nic/utils/print",
    ],
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
