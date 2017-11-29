package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_binary(
    name = "hal",
    srcs = [
        "main.cc",
    ],
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lz",
        "-lprotobuf",
        "-rdynamic",
        "-lsnort",
    ],
    deps = [
        ":hal_src",
        # PI
        "//nic/fte:fte",
        "//nic/hal/periodic",
        "//nic/hal/svc:hal_svc",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/utils/host_mem:host_mem",
        "//nic/hal/plugins:plugins",
        "//nic/hal/plugins/proxy:proxyplugin",
        "//nic/hal/lkl:lkl_api",
        "//nic:lkl",
        "//nic:halproto",
        "//nic:grpc",
        "//nic/third-party/isc-dhcp:isc-dhcp",  # TODO(move to eplearn plugin)
        "//nic:snort3",  # TODO(move to app_redir plugin)

        # PD
        "//nic/hal/pd/common:pdcommon",
        "//nic/hal/pd/utils/directmap",
        "//nic/hal/pd/utils/hash",
        "//nic/hal/pd/utils/tcam",
        "//nic/hal/pd/utils/flow",
        "//nic/hal/pd/utils/met",
        "//nic/hal/pd/utils/acl_tcam",
        "//nic/hal/pd/iris/p4pd:p4pdapi",
        "//nic/hal/pd/iris",
        "//nic:capricsr_int",
        "//nic/hal/pd/capri",
        "//nic/hal/pd/pal",
        "//nic/hal/pd/control",
        "//nic/model_sim:model_client",
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
    deps = [
        "//nic/hal/periodic",
        "//nic/include:base_includes",
        "//nic/hal:hal_includes",
        "//nic:asic_includes",
        "//nic/hal/pd/common:pdcommon",
        "//nic/p4/include:p4_common_includes",
        "//nic/p4/nw/include:p4_nw_includes",
        "//nic/asm/cpu-p4plus/include:cpu_p4plus_includes",
        "//nic/hal/plugins:plugins_includes",
        "//nic/hal/plugins/network:network_includes",
        "//nic/hal/plugins/proxy:proxyplugin_includes",
        "//nic/hal/plugins/app_redir:app_redir_includes",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/utils/thread:thread",
        "//nic/utils/print:print",
        "//nic/utils/slab:slab",
        "//nic/utils/block_list:block_list",
        "//nic/utils/indexer:indexer",
        "//nic/utils/ht:ht",
        "//nic:gen_includes",
        "//nic:gen_proto_includes",
        "//nic/hal/pd/capri:capri_includes",
        "//nic/hal/pd/iris:iris_includes",
        "//nic/utils/host_mem:host_mem_includes",
        "//nic/third-party/spdlog",
        "//nic/hal/tls:tls_api",
        "//nic/utils/fsm:fsm",
        "//nic/fte:fte_includes",
        "//nic/utils/eventmgr:eventmgr",
        "//nic/utils/catalog",
    ],
)

cc_library(
    name = "hal_includes",
    srcs = [],
    hdrs = ["hal.hpp",] + 
            glob(["src/*.hpp",]),
)

cc_library(
    name = "hal_utils_includes",
    srcs = [],
    hdrs = ["src/utils.hpp"],
)
