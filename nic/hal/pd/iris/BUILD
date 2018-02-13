package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_binary(
    name = "libiris.so",
    linkshared = 1,
    deps = [
            "//nic/hal/pd/iris:pd_iris"
    ],
)

cc_library(
    name = "pd_iris",
    srcs = [] + glob(["*.cc"]),
    hdrs = [] + glob(["*.hpp"]) + glob(["*.h"]),
    copts = sdk_copts,
    deps = [
        # PD Cleanup Only
        #"//nic/hal/pd/iris:p4pdapi",
        "//nic:iris_p4pd",
        "//nic:p4pluspd_txdma",
        "//nic:p4pluspd_rxdma",
        # -----
        "//nic:asic_includes",
        "//nic:gen_includes",
        "@sdk//:sdk_headers",
        "//nic/hal:hal_includes",
        #"//nic/hal:hal_src",
        "//nic/hal/pd/capri",
        "//nic/hal/pd/capri:capri_includes",
        "//nic/hal/pd:pdcommon",
        "//nic/hal/pd/p4pd:p4pd",
        "//nic/hal/pd/utils/acl_tcam",
        "//nic/hal/pd/utils/flow",
        "//nic/hal/pd/utils/met",
        "@sdk//obj:sdk_tcam",
        "@sdk//obj:sdk_hash",
        "@sdk//obj:sdk_directmap",
        "@sdk//obj:sdk_catalog",
        "//nic/include:base_includes",
        "//nic/p4/nw/include:p4_nw_includes",
        "//nic:spdlog",
        "//nic:grpc_includes",
        "//nic/p4/include:p4_common_includes",
        "//nic/hal/pd/libs/wring:wringpd",
        "//nic/hal/pd/libs/cpupkt:cpupkt",
        "//nic/hal/pd/asicpd:asicpd",
    ],
    alwayslink = 1,
)

cc_library(
    name = "iris_includes",
    srcs = [],
    hdrs = ["eth_pd.hpp",
            "rdma_pd.hpp",
            "hal_state_pd.hpp",
            "rw_pd.hpp",
            "tnnl_rw_pd.hpp",
            "acl_pd.hpp",
            "p4plus_pd_api.h",
            "if_pd_utils.hpp"],
    deps = [
        "//nic:gen_includes",
        "//nic/hal/pd/utils/acl_tcam:acl_tcam_includes",
        "//nic/hal/pd/utils/flow:flow_includes",
        "//nic/hal/pd/utils/met:met_includes",
        "@sdk//obj:sdk_tcam",
        "@sdk//obj:sdk_hash",
        "@sdk//obj:sdk_directmap",
        "//nic/utils/bitmap",
        "//nic/utils/bm_allocator",
        "//nic/p4/rdma/include:p4_rdma_includes",
    ],
)
