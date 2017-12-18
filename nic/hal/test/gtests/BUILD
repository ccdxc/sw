package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_test(
    name = "network_test",
    srcs = ["network_test.cc"],
    deps = gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)
cc_test(
    name = "acl_test",
    srcs = ["acl_test.cc"],
    deps = gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "nwsec_test",
    srcs = ["nwsec_test.cc"],
    deps = [
        "//nic/hal/plugins/sfw:sfw",
    ] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "tunnelif_test",
    srcs = ["tunnelif_test.cc"],
    deps = gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)
cc_test(
    name = "vrf_test",
    srcs = ["vrf_test.cc"],
    deps = gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "l2seg_test",
    srcs = ["l2seg_test.cc"],
    deps = gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "endpoint_test",
    srcs = ["endpoint_test.cc"],
    deps = gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "internal_test",
    srcs = ["internal_test.cc"],
    deps = gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)


cc_test(
    name = "bm_allocator_test",
    srcs = ["bm_allocator_test.cc"],
    deps = [] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "cpuif_test",
    srcs = ["cpuif_test.cc"],
    deps = [] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "enicif_test",
    srcs = ["enicif_test.cc"],
    deps = [] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "lif_manager_test",
    srcs = ["lif_manager_test.cc"],
    deps = [] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "lif_test",
    srcs = ["lif_test.cc"],
    deps = [] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "session_test",
    srcs = ["session_test.cc"],
    deps = [] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "tcpcb_test",
    srcs = ["tcpcb_test.cc"],
    deps = [] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "tlscb_test",
    srcs = ["tlscb_test.cc"],
    deps = [] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "uplinkif_test",
    srcs = ["uplinkif_test.cc"],
    deps = [] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "uplinkpc_test",
    srcs = ["uplinkpc_test.cc"],
    deps = [] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "scheduler_tx_test",
    srcs = ["scheduler_tx_test.cc"],
    deps = [] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "gft_test",
    srcs = ["gft_test.cc"],
    deps = [
        "//nic/p4/gft/include:p4_gft_includes",
        "//nic:gft_p4pd"] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "appid_test",
    srcs = ["appid_test.cc"],
    deps = [
        "//nic/hal/plugins/app_redir:app_redir",
        "//nic:snort3"
    ] + gl_deps_list,
    linkopts = ["-lsnort"] + 
        gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "qos_test",
    srcs = ["qos_test.cc"],
    deps = [] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "copp_test",
    srcs = ["copp_test.cc"],
    deps = [] + gl_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)
