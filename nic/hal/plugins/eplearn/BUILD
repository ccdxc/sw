package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

common_deps = [
    "//nic/hal:hal_hdrs",
    "//nic/hal/src/nw:nw_includes",
    "//nic/utils/fsm:fsm",
    "//nic/fte:fte_includes",
    "//nic/include:base_includes",
    "//nic/hal/pd:pdcommon",
    "@sdk//obj:sdk_ht",
    "@sdk//obj:sdk_catalog",
    "@sdk//obj:sdk_indexer",
    "@sdk//obj:sdk_slab",
    "@sdk//obj:sdk_twheel",
    "//nic/utils/block_list:block_list",
    "//nic:spdlog",
    "//nic:gen_proto_includes",
    "//nic:grpc",
    "//nic:grpc_includes",
    "//nic:google_includes",
    "//nic:boost",
    "//nic/hal/periodic:periodic",
]

cc_binary(
    name = "libeplearn.so",
    linkshared = 1,
    deps = [
        "//nic/hal/plugins/eplearn:ep_learn",
        "//nic/hal/obj:isc-dhcp",
        ],
)

cc_library(
    name = "ep_learn",
    srcs = glob(["*.cc"]) + glob(["dhcp/*.cc"]) + glob(["arp/*.cc"]) + glob(["common/*.cc"]),
    hdrs = glob(["dhcp/*.hpp"]) + glob(["arp/*.hpp"]) + glob(["common/*.hpp"]),
    deps = [
        "@sdk//obj:sdk_slab",
        "//nic/hal/third-party/isc-dhcp:isc-dhcp",
        "//nic/utils/bm_allocator:bm_allocator",
        "//nic/hal/pd:hal_pd_includes",
        "//nic:gen_proto_includes",
        "//nic/hal:hal_hdrs",
        "//nic/hal/src/nw:nw_includes",
        "//nic/utils/fsm:fsm",
        "//nic/fte:fte_includes",
        "//nic/include:base_includes",
        "//nic/utils/block_list:block_list",
        "//nic:grpc_includes",
        "//nic:google_includes",
        "//nic:boost",
        ":eplearn_includes",
    ],
    alwayslink = 1,
    copts = sdk_copts,
)

cc_library(
    name = "eplearn_includes",
    hdrs = [
        "eplearn.hpp",
        "arp/arp_learn.hpp",
        "dhcp/dhcp_learn.hpp",
    ],
)

cc_library(
    name = "ep_learn_common",
    srcs = glob(["common/*.cc"]),
    hdrs = glob(["common/*.hpp"]) + ["eplearn.hpp", "dhcp/dhcp_trans.hpp", "arp/arp_trans.hpp"],
    deps = common_deps,
    copts = sdk_copts,
    alwayslink = 1,
)

cc_test(
    name = "dhcp_test",
    srcs = glob(["test/dhcp/*.cc"]) + [
        "test/hal_mock.cc",
    ],
    tags = ["manual", "gtests"],
    copts = sdk_copts,
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    deps = gl_deps_list + [":ep_learn"] + test_deps_list,
)

cc_test(
    name = "arp_test",
    srcs = glob(["test/arp/*.cc"]) + [
        "test/hal_mock.cc",
    ],
    tags = ["manual", "gtests"],
    copts = sdk_copts,
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    deps = gl_deps_list + [":ep_learn"] + test_deps_list,
)
