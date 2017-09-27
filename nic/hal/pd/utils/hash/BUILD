package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "hash",
    srcs = [
        "hash.cc",
        "hash.hpp",
        "hash_entry.cc",
        "hash_entry.hpp",
    ],
    hdrs = [
        "hash.hpp",
        "hash_entry.hpp",
    ],
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        "//nic/hal/pd/utils/tcam",
        "//nic/include:base_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/indexer",
        "//nic/utils/thread",
    ],
)

cc_library(
    name = "hash_includes",
    srcs = [],
    hdrs = ["hash.hpp"],
)

cc_test(
    name = "hash_test",
    srcs = ["test/hash_test.cc"],
    linkopts = [
        "-lzmq",
    ],
    deps = [
        ":hash",
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
		"//nic/hal/pd/utils/p4pd_mock:p4pd_mock",
    ],
)
