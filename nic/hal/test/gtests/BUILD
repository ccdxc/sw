package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "gl_deps_list")
load("//nic/bazel:common_deps.bzl", "gl_linkopts_list")
load("//nic/bazel:common_deps.bzl", "sdk_copts")
load("//nic/bazel:common_deps.bzl", "test_deps_list")

cc_test(
    name = "network_test",
    srcs = ["network_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)
cc_test(
    name = "acl_test",
    srcs = ["acl_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "nwsec_test",
    srcs = ["nwsec_test.cc"],
    deps = [
        "//nic/hal/plugins/sfw:sfw",
        "//nic/hal/plugins/app_redir:app_redir_includes",
        "//nic/hal/plugins/alg_utils:alg_utils",
    ] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "tunnelif_test",
    srcs = ["tunnelif_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)
cc_test(
    name = "vrf_test",
    srcs = ["vrf_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "l2seg_test",
    srcs = ["l2seg_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "endpoint_test",
    srcs = ["endpoint_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "internal_test",
    srcs = ["internal_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "bm_allocator_test",
    srcs = ["bm_allocator_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "cpuif_test",
    srcs = ["cpuif_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "enicif_test",
    srcs = ["enicif_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "lif_manager_test",
    srcs = ["lif_manager_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "lif_test",
    srcs = ["lif_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "session_test",
    srcs = ["session_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "tcpcb_test",
    srcs = ["tcpcb_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "tlscb_test",
    srcs = ["tlscb_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "uplinkif_test",
    srcs = ["uplinkif_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "uplinkpc_test",
    srcs = ["uplinkpc_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "scheduler_tx_test",
    srcs = ["scheduler_tx_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "gft_test",
    srcs = ["gft_test.cc"],
    deps = [
        "//nic/p4/gft/include:p4_gft_includes",
        "//nic/hal/pd/pd_stub",
        "//nic/hal/pd/gft:pd_gft",
        "//nic/hal:hal_src",
        "//nic/fte:fte",
        "//nic/hal/plugins:plugins",
        "//nic/hal/plugins/proxy:proxyplugin",
        "//nic/hal/lkl:lkl_api",
        "//nic:lkl",
        #"//nic/hal/test/utils:haltestutils",
        "//nic:gft_p4pd",
        "//nic/model_sim:model_client",
        "//nic/hal/pd/capri",
        "//nic/utils/trace",
        "//nic:grpc",
        "//nic:gen_proto_includes",
        "//nic:libprotobuf",
        "//nic:halproto",
        "//nic:p4pluspd_txdma",
        "//nic:p4pluspd_rxdma",
        "//nic:asic_libs",
        "@sdk//obj:sdk_logger",
        "@sdk//obj:sdk_pal",
        "@sdk//obj:sdk_catalog",
        "//nic/hal/pd/p4pd:p4pd",
        "//:gtest",
        ],
    linkopts = [] + gl_linkopts_list,
    tags = ["gft"],
    copts = sdk_copts,
)

cc_test(
    name = "appid_test",
    srcs = ["appid_test.cc"],
    deps = [
        "//nic/hal/plugins/app_redir:app_redir",
        "//nic:snort3",
    ] + gl_deps_list + test_deps_list,
    linkopts = ["-lsnort"] + 
        gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "qos_test",
    srcs = ["qos_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)

cc_test(
    name = "copp_test",
    srcs = ["copp_test.cc"],
    deps = [] + gl_deps_list + test_deps_list,
    linkopts = [] + gl_linkopts_list,
    copts = sdk_copts,
)
