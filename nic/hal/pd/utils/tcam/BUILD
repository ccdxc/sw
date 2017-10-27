package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "tcam",
    srcs = ["tcam.cc", 
            "tcam.hpp",
            "tcam_entry.cc",
            "tcam_entry.hpp"],
    hdrs = ["tcam.hpp", 
            "tcam_entry.hpp"],
    deps = ["//nic/include:base_includes", 
            "//nic/third-party/spdlog:spdlog", 
            "//nic/utils/indexer:indexer", 
            "//nic/hal/pd:hal_pd_includes", 
            "//nic/utils/thread:thread",
            "//nic/hal/pd/utils/acl_tcam:acl_tcam_includes"],
)

cc_library(
    name = "tcam_includes",
    srcs = [],
    hdrs = ["tcam.hpp", 
            "tcam_entry.hpp"],
)
cc_test(
    name = "tcam_test",
    srcs = ["test/tcam_test.cc"],
    linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lprotobuf"
    ],
    deps = [
        ":tcam",
        "//:gtest",
        "//nic/utils/trace",
        "//nic/fte:fte",
        "//nic/hal/plugins/network:network",
        "//nic/hal/plugins/classic:classic",
        "//nic/hal/plugins/proxy:proxyplugin",
        "//nic/utils/host_mem:host_mem",
        "//nic/hal/test/utils:haltestutils",
        "//nic/hal/lkl:lkl_api",
        "//nic:lkl",

        # PD
        "//nic/hal/pd/common:pdcommon",
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
        "//nic/hal/pd/utils/p4pd_mock:p4pd_mock"
    ],
)
