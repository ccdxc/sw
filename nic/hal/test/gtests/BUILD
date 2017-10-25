package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

gl_deps = [
        "//nic/hal:hal_src",
        # PI
        "//nic/fte:fte",
        "//nic/hal/periodic",
        "//nic/hal/svc:hal_svc",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/hal/plugins/network:network",
        "//nic/hal/plugins/proxy:proxyplugin",
        "//nic/utils/host_mem:host_mem",
        "//nic/hal/test/utils:haltestutils",
        "//nic/hal/lkl:lkl_api",
        "//nic:lkl",
        "//nic:halproto",
        "//nic:grpc",

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
        "//nic/hal/pd/control",
        "//nic/model_sim:model_client",
        "//nic:p4pd",
        "//nic:p4pluspd_txdma",
        "//nic:p4pluspd_rxdma",
        "//nic:asic_libs",

        # External
        "//:gtest",
        ]

gl_linkopts = [
    "-lzmq",
    "-lpthread",
    "-pthread",
    "-lz",
    "-lprotobuf"
    ]

cc_test(
    name = "network_test",
    srcs = ["network_test.cc"],
    deps = gl_deps,
    linkopts = [] + gl_linkopts,
)
cc_test(
    name = "acl_test",
    srcs = ["acl_test.cc"],
    deps = gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "nwsec_test",
    srcs = ["nwsec_test.cc"],
    deps = gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "tunnelif_test",
    srcs = ["tunnelif_test.cc"],
    deps = gl_deps,
    linkopts = [] + gl_linkopts,
)
cc_test(
    name = "tenant_test",
    srcs = ["tenant_test.cc"],
    deps = gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "l2seg_test",
    srcs = ["l2seg_test.cc"],
    deps = gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "endpoint_test",
    srcs = ["endpoint_test.cc"],
    deps = gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "internal_test",
    srcs = ["internal_test.cc"],
    deps = gl_deps,
    linkopts = [] + gl_linkopts,
)


cc_test(
    name = "bm_allocator_test",
    srcs = ["bm_allocator_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "buf_pool_test",
    srcs = ["buf_pool_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "cpuif_test",
    srcs = ["cpuif_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "enicif_test",
    srcs = ["enicif_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "lif_manager_test",
    srcs = ["lif_manager_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "lif_test",
    srcs = ["lif_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "policer_test",
    srcs = ["policer_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "queue_test",
    srcs = ["queue_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "session_test",
    srcs = ["session_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "tcpcb_test",
    srcs = ["tcpcb_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "tlscb_test",
    srcs = ["tlscb_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "uplinkif_test",
    srcs = ["uplinkif_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "uplinkpc_test",
    srcs = ["uplinkpc_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)

cc_test(
    name = "scheduler_tx_test",
    srcs = ["scheduler_tx_test.cc"],
    deps = [] + gl_deps,
    linkopts = [] + gl_linkopts,
)
