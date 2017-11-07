package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")

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
		"//nic/utils/catalog:catalog",
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
	    "ep_learn/common/trans.hpp",
	    "ep_learn/dhcp/dhcp_trans.hpp",
	    "ep_learn/arp/arp_trans.hpp"],
)

cc_library(
    name = "dhcp_learn",
    srcs = glob(["ep_learn/dhcp/*.cc"]),
    hdrs = glob(["ep_learn/dhcp/*.hpp"]) + ["ep_learn/common/trans.hpp"],
    deps = common_deps + ["//nic/third-party/isc-dhcp"],
    alwayslink = 1,
)

cc_library(
    name = "arp_learn",
    srcs = glob(["ep_learn/arp/*.cc"]),
    hdrs = glob(["ep_learn/arp/*.hpp"]) + ["ep_learn/common/trans.hpp"],
    deps = common_deps,
    alwayslink = 1,
)

cc_test(
    name = "dhcp_test",
    srcs = glob(["test/ep_learn/dhcp/*.cc"]) +
		["test/hal_mock.cc"],
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    deps = gl_deps_list,
)

cc_test(
    name = "arp_test",
    srcs = glob(["test/ep_learn/arp/*.cc"]) +
		["test/hal_mock.cc"],
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    deps = gl_deps_list,
)
