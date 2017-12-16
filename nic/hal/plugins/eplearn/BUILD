package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")

common_deps = [
    "//nic/hal:hal_hdrs",
    "//nic/utils/fsm:fsm",
    "//nic/fte:fte_includes",
    "//nic/include:base_includes",
    "//nic/p4/nw/include:p4_nw_includes",
    "//nic/utils/catalog:catalog",
    "//nic/hal/pd/common:pdcommon",
    #"//nic:sdk_ht",
    #"//nic:sdk_slab",
    #"//nic:sdk_indexer",
    "@sdk//obj:sdk_ht",
    "@sdk//obj:sdk_indexer",
    "@sdk//obj:sdk_slab",
    "//nic/utils/thread:thread",
    "//nic/utils/twheel:twheel",
    "//nic/utils/block_list:block_list",
    "//nic/third-party/spdlog",
    "//nic:gen_proto_includes",
    "//nic:grpc",
    "//nic:gen_includes",
]

cc_library(
    name = "eplearn",
    srcs = glob(["*.cc"]) + glob(["dhcp/*.cc"]) + glob(["arp/*.cc"]) + glob(["common/*.cc"]),
    deps = [
        ":arp_learn",
        ":dhcp_learn",
        ":ep_learn_common",
    ],
    alwayslink = 1,
    copts = sdk_copts,
)

cc_library(
    name = "eplearn_includes",
    hdrs = [
        "arp/arp_learn.hpp",
        "dhcp/dhcp_learn.hpp",
    ],
)

cc_library(
    name = "ep_learn_common",
    srcs = glob(["common/*.cc"]),
    hdrs = glob(["common/*.hpp"]),
    deps = common_deps,
    copts = sdk_copts,
    alwayslink = 1,
)

cc_library(
    name = "dhcp_learn",
    srcs = glob(["dhcp/*.cc"]),
    hdrs = glob(["dhcp/*.hpp"]),
    deps = common_deps + [
        "//nic/third-party/isc-dhcp",
        ":ep_learn_common",
    ],
    copts = sdk_copts,
    alwayslink = 1,
)

cc_library(
    name = "arp_learn",
    srcs = glob(["arp/*.cc"]),
    hdrs = glob(["arp/*.hpp"]),
    deps = common_deps + [":ep_learn_common"],
    copts = sdk_copts,
    alwayslink = 1,
)

cc_test(
    name = "dhcp_test",
    srcs = glob(["test/dhcp/*.cc"]) + [
        "test/hal_mock.cc",
    ],
    copts = sdk_copts,
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    deps = gl_deps_list + [":eplearn"],
)

cc_test(
    name = "arp_test",
    srcs = glob(["test/arp/*.cc"]) + [
        "test/hal_mock.cc",
    ],
    copts = sdk_copts,
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    deps = gl_deps_list + [":eplearn"],
)
