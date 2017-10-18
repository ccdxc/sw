package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "flow",
    srcs = ["flow.cc", 
            "flow.hpp",
            "flow_entry.cc",
            "flow_entry.hpp",
            "flow_hint_group.cc",
            "flow_hint_group.hpp",
            "flow_spine_entry.cc",
            "flow_spine_entry.hpp",
            "flow_table_entry.cc",
            "flow_table_entry.hpp"],
    hdrs = ["flow.hpp",
            "flow_entry.hpp",
            "flow_hint_group.hpp",
            "flow_spine_entry.hpp",
            "flow_table_entry.hpp"],
    deps = [
            "//nic:gen_includes",
            "//nic/include:base_includes", 
            "//nic/third-party/spdlog:spdlog", 
            "//nic/utils/indexer:indexer", 
            "//nic/hal/pd:hal_pd_includes", 
            "//nic/utils/thread:thread"],
)

cc_library(
    name = "flow_includes",
    srcs = [],
    hdrs = ["flow.hpp",
            "flow_entry.hpp",
            "flow_hint_group.hpp",
            "flow_spine_entry.hpp",
            "flow_table_entry.hpp"],
)

cc_test(
    name = "flow_test",
    srcs = ["test/flow_test.cc"],
    deps = [
        ":flow",
        "//:gtest",
        "//nic/utils/trace",
        "//nic/fte:fte",
        "//nic/hal/plugins/network:network",
        "//nic/hal/plugins/proxy:proxyplugin",
        "//nic/utils/host_mem:host_mem",
        "//nic/hal/test/utils:haltestutils",
        "//nic/hal/lkl:lkl_api",
        "//nic:lkl",

        # PD
        "//nic/hal/pd/common:pdcommon",
        "//nic/hal/pd/utils/hash:hash",
        "//nic/hal/pd/utils/tcam:tcam",
        "//nic/hal/pd/utils/met:met",
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
   linkopts = [
        "-lzmq",
        "-lpthread",
        "-pthread",
        "-lprotobuf"
     ],
)
