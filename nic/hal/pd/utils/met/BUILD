package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "met",
    srcs = [
        "met.cc",
        "met.hpp",
        "repl_entry.cc",
        "repl_entry.hpp",
        "repl_list.cc",
        "repl_list.hpp",
        "repl_table_entry.cc",
        "repl_table_entry.hpp",
    ],
    hdrs = [
        "met.hpp",
        "repl_entry.hpp",
        "repl_list.hpp",
        "repl_table_entry.hpp",
    ],
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        "//nic/hal/pd/capri",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/indexer",
        "//nic/utils/thread",
    ],
)

cc_library(
    name = "met_includes",
    srcs = [],
    hdrs = [
        "met.hpp",
        "repl_entry.hpp",
        "repl_list.hpp",
        "repl_table_entry.hpp",
    ],
)

cc_test(
    name = "met_test",
    srcs = ["test/met_test.cc"],
    linkopts = [
        "-lzmq",
    ],
    deps = [
        ":met",
        "//:gtest",
        "//nic/utils/trace",
        "//nic/fte",
        "//nic/hal/plugins/network",
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
        "//nic/model_sim:model_client",
        "//nic:p4pd",
        "//nic:p4pdtcprx",
        "//nic:p4pdtcptx",
        "//nic:p4pluspd_txdma",
        "//nic:p4pluspd_rxdma",
        "//nic:asic_libs",
        "//nic/hal/pd/utils/p4pd_mock",
    ],
)
