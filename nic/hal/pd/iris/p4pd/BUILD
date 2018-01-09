package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "p4pdapi",
    srcs = [
        "p4pd_api.cc",
        "p4pd_global_api.cc",
        "p4pd_init.cc",
        "p4pluspd_rxdma_api.cc",
        "p4pluspd_txdma_api.cc",
    ],
    hdrs = glob([
        "*.hpp",
        "*.h",
    ]),
    copts = sdk_copts,
    deps = [
        "//nic:gen_includes",
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_slab",
        "@sdk//obj:sdk_list",
        "//nic/hal/periodic:periodic",
        "//nic/hal/lib:hal_handle_includes",
        "//nic/hal/pd:hal_pd_includes",
        "//nic/hal/pd/capri:capri_includes",
        "//nic/hal/pd/iris:iris_includes",
        "//nic/hal/pd/utils/acl_tcam",
        #"//nic/hal/pd/utils/tcam",
        "@sdk//obj:sdk_tcam",
        "//nic/include:base_includes",
        "//nic/p4/include:p4_common_includes",
        "//nic/p4/nw/include:p4_nw_includes",
        "//nic/p4/rdma/include:p4_rdma_includes",
        "//nic/third-party/spdlog",
        "//nic/utils/bm_allocator",
        "//nic:gen_proto_includes",
    ],
    alwayslink = 1,
)
