package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")

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

cc_test(
    name = "directmap_test",
    srcs = ["test/directmap_test.cc"],
    deps = gl_deps_list,
    linkopts = gl_linkopts_list,

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
