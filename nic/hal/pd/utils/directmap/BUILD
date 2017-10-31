package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "directmap",
    srcs = [
        "directmap.cc",
        "directmap.hpp",
        "directmap_entry.cc",
        "directmap_entry.hpp",
    ],
    hdrs = ["directmap.hpp"],
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/indexer",
        "//nic/utils/thread",
        "//nic/hal/pd/iris:iris_includes",
        "//nic/utils/slab:slab",
        "//nic/utils/bm_allocator:bm_allocator",
    ],
)

cc_library(
    name = "directmap_includes",
    srcs = [],
    hdrs = ["directmap.hpp",
            "directmap_entry.hpp"],
)


gl_deps = [
        "//nic/hal:hal_src",
        # PI
        "//nic/fte:fte",
        "//nic/hal/periodic",
        "//nic/hal/svc:hal_svc",
        "//nic/utils/trace",
        "//nic/utils/print",
        "//nic/hal/plugins/network:network",
        "//nic/hal/plugins/classic:classic",
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
    name = "directmap_test",
    srcs = ["test/directmap_test.cc"],
    deps = gl_deps,
    linkopts = gl_linkopts,

    #deps = [
    #    ":directmap",
    #    "//:gtest",
    #    "//nic/utils/trace",
    #    "//nic/fte:fte",
    #    "//nic/hal/plugins/network:network",
    #    "//nic/hal/plugins/classic:classic",
    #    "//nic/hal/plugins/proxy:proxyplugin",
    #    "//nic/utils/host_mem:host_mem",
    #    "//nic/hal/test/utils:haltestutils",
    #    "//nic/hal/lkl:lkl_api",
    #    "//nic:lkl",

    #    # PD
    #    "//nic/hal/pd/common:pdcommon",
    #    "//nic/hal/pd/utils/hash:hash",
    #    "//nic/hal/pd/utils/tcam:tcam",
    #    "//nic/hal/pd/utils/flow:flow",
    #    "//nic/hal/pd/utils/met:met",
    #    "//nic/hal/pd/iris/p4pd:p4pdapi",
    #    "//nic/hal/pd/iris",
    #    "//nic:capricsr_int",
    #    "//nic/hal/pd/capri",
    #    "//nic/hal/pd/control",
    #    "//nic/model_sim:model_client",
    #    "//nic:p4pd",
    #    "//nic:p4pluspd_txdma",
    #    "//nic:p4pluspd_rxdma",
    #    "//nic:asic_libs",
    #    "//nic/hal/pd/utils/p4pd_mock:p4pd_mock"
    #   ],
   #linkopts = [
   #      "-lzmq",
   #      "-lpthread",
   #      "-pthread",
   #      "-lprotobuf"
   #  ],
)
