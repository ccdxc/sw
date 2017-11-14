package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")

common_deps = [
		"//nic/hal:hal_hdrs",
		"//nic/utils/fsm:fsm",
		"//nic/fte:fte_includes",
		"//nic/include:base_includes",
    "//nic/utils/catalog:catalog",
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

cc_library(                         
    name = "eplearn",
    srcs = glob(["*.cc"]) + glob(["dhcp/*.cc"]) + glob(["arp/*.cc"]),
    hdrs = glob(["*.hpp"]) + glob(["dhcp/*.hpp"]) + glob(["arp/*.hpp"]) + ["common/trans.hpp"],
    deps = common_deps + ["//nic/third-party/isc-dhcp"],
    alwayslink = 1,
)

cc_test(
    name = "dhcp_test",
    srcs = glob(["test/dhcp/*.cc"]) + ["test/hal_mock.cc"],
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    deps = gl_deps_list + ["eplearn"],
)

cc_test(
    name = "arp_test",
    srcs = glob(["test/arp/*.cc"]) + ["test/hal_mock.cc"],
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    deps = gl_deps_list + ["eplearn"],
)
