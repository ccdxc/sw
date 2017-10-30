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
        "//nic/hal/plugins/classic:classic",
        "//nic/utils/host_mem:host_mem",
        "//nic:gen_proto_includes",
        "//nic/hal/test/utils:haltestutils",
        "//nic/hal/lkl:lkl_api",
        "//nic:lkl",

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
        "//nic/model_sim:model_client",
        "//nic:p4pd",
        "//nic:p4pluspd_txdma",
        "//nic:p4pluspd_rxdma",
        "//nic:asic_libs",

        # External
        "//:gtest",
        ]


common_deps = [
		"//nic/hal:hal_hdrs",
		"//nic/utils/fsm:fsm",
		"//nic/fte:fte_includes",
		"//nic/include:base_includes",
                "//nic/hal/pd/common:pdcommon",
		"//nic/utils/ht:ht",
		"//nic/utils/slab:slab",
		"//nic/utils/indexer:indexer",
		"//nic/utils/thread:thread",
		"//nic/utils/twheel:twheel",
		"//nic/third-party/spdlog",
                "//nic:gen_proto_includes",
		"//nic:gen_includes",
]

gl_linkopts = [
    "-lzmq",
    "-lpthread",
    "-pthread",
    "-lz",
    "-lprotobuf"
    ]

cc_library(
    name = "network",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//nic:gen_includes",
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        #"//nic/proto:types_proto",
        #"//nic/proto/hal:hal_proto",
        "//nic/third-party/spdlog",
        "//nic:gen_proto_includes",
        "//nic/hal/plugins/network/alg",
        ":dhcp_learn",
        ":arp_learn",
    ],
    alwayslink = 1,
)

cc_library(
    name = "network_includes",
    hdrs = ["net_plugin.hpp",
	    "ep_learn/dhcp/dhcp_trans.hpp",
	    "ep_learn/arp/arp_trans.hpp"],
)

cc_library(
    name = "dhcp_learn",
    srcs = glob(["ep_learn/dhcp/*.cc"]),
    hdrs = glob(["ep_learn/dhcp/*.hpp"]),
    deps = common_deps + ["//nic/third-party/isc-dhcp"],
    alwayslink = 1,
)

cc_library(
    name = "arp_learn",
    srcs = glob(["ep_learn/arp/*.cc"]),
    hdrs = glob(["ep_learn/arp/*.hpp"]),
    deps = common_deps,
    alwayslink = 1,
)

cc_test(
    name = "dhcp_test",
    srcs = glob(["test/ep_learn/dhcp/*.cc"]) +
		["test/hal_mock.cc"],
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts,
    deps = gl_deps,
)

cc_test(
    name = "arp_test",
    srcs = glob(["test/ep_learn/arp/*.cc"]) +
		["test/hal_mock.cc"],
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts,
    deps = gl_deps,
)
