package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "test_deps_list")


cc_library(
    name = "firewall_includes",
    srcs = glob(["cfg/*.hpp"]),
    hdrs = glob(["cfg/*.hpp"]),
    deps = [
               "//nic/hal/plugins/cfg/nw:nw_includes",
           ]
)

cc_library(
    name = "sfw",
    srcs = glob(["*.cc", "cfg/*.cc"]),
    hdrs = glob(["*.hpp", "cfg/*.hpp"]),
    deps = [
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic/model_sim/include:modelsim_includes",
        "//nic:spdlog",
        "//nic/hal/plugins/app_redir:app_redir_includes",
        "//nic/hal/plugins/alg_utils:alg_utils_includes",
        "//nic/hal/plugins/alg_utils:alg_utils",
        "//nic/hal:hal_includes",
        "//nic:gen_proto_includes",
        "//nic:hal_svc_gen_includes",
        "//nic:hal_gen_includes",
        "//nic/hal/core:periodic_includes",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/utils/block_list",
        "//nic/fte:fte_includes",
        "//nic/hal/plugins/cfg/nw:nw_includes",
        "//nic/hal/plugins/cfg/mcast:mcast_includes",
        "//nic/hal/plugins/cfg/aclqos:aclqos_includes",
        "//nic/hal/plugins/cfg/telemetry:telemetry_includes",
        "//nic/utils/bm_allocator",
        "//nic/hal/plugins/cfg/ipsec:ipsec_includes",
        "//nic/hal/plugins/cfg/gft:gft_includes",
        "//nic/hal/pd:hal_pd_includes",
    ],
    alwayslink = 1,
    copts = sdk_copts,
)

cc_library(
    name = "sfw_includes",
    hdrs = ["core.hpp"],
)

cc_library(
    name = "sfw_pkt_utils",
    srcs = glob(["sfw_pkt_utils.cc"]),
    hdrs = glob(["sfw_pkt_utils.hpp"]),
    deps = [
        "//nic/fte",
        "//nic/include:base_includes",
        "//nic:spdlog",
    ],
)

cc_test(
    name = "sfw_test",
    srcs = glob(["test/*.cc", "test/*.hpp"]),
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    deps = [
        "//nic/fte:fte_base_test",
	] + gl_deps_list + test_deps_list,
    copts = sdk_copts,
    tags = ["gtests"],
)

cc_test(
    name = "nwsec_policy_test",
    srcs = glob(["test/*.cc"]),
    deps = [
        "//nic/hal/plugins/sfw:sfw",
        "//nic/hal/plugins/app_redir:app_redir_includes",
        "//nic/hal/plugins/alg_utils:alg_utils",
        "//nic/fte:fte_base_test",
    ] + gl_deps_list + test_deps_list,
    linkopts = ["-L/usr/local/lib -ltins"] + gl_linkopts_list,
    tags=['gtests'],
    copts = sdk_copts,
)
