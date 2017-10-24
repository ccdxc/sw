package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "p4pdapi",
    srcs = ["p4pd_init.cc", 
            "p4pd_api.cc",
            "p4pluspd_rxdma_api.cc",
            "p4pluspd_txdma_api.cc",
            "p4pd_global_api.cc"],
    hdrs = glob(["*.hpp", "*.h"]),
    deps = ["//nic/include:base_includes", 
            "//nic:gen_includes",
            "//nic/p4/nw/include:p4_nw_includes",
            "//nic/p4/rdma/include:p4_rdma_includes",
            "//nic/p4/include:p4_common_includes",
            "//nic/third-party/spdlog:spdlog",
            "//nic/hal/pd:hal_pd_includes",
            "//nic/hal/pd/iris:iris_includes",
            "//nic/hal/pd/capri:capri_includes",
            "//nic/hal/pd/utils/tcam:tcam",
            "//nic/hal/pd/utils/acl_tcam:acl_tcam",
            "//nic/utils/slab:slab",
            "//nic/utils/ht:ht",
            "//nic/utils/bm_allocator",
            ],
    alwayslink = 1,
)
