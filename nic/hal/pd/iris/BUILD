package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "iris",
    srcs = [] + glob(["*.cc"]),
    hdrs = [] + glob(["*.hpp"]) + glob(["*.h"]),
    deps = [
            "//nic/include:base_includes",
            "//nic/third-party/spdlog:spdlog",
            "//nic/hal:hal_src",
            "//nic:asic_includes",
            "//nic/hal/pd/common:pdcommon",
            "//nic/hal/pd/capri:capri",
            "//nic/p4/nw/include:p4_nw_includes",
            "//nic/hal/pd/utils/directmap:directmap",
            "//nic/hal/pd/utils/hash:hash",
            "//nic/hal/pd/utils/tcam:tcam",
            "//nic/hal/pd/utils/flow:flow",
            "//nic/hal/pd/utils/met:met",
            "//nic/hal/pd/utils/acl_tcam:acl_tcam",
            "//nic:gen_includes",
            "//nic/hal/pd/iris/p4pd:p4pdapi",
            "//nic/hal/pd/capri:capri_includes",
            "//nic/hal/pd/control:control",
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
            "if_pd_utils.hpp"],
    deps = [
            "//nic/hal/pd/utils/directmap:directmap_includes",
            "//nic/hal/pd/utils/met:met_includes",
            "//nic/hal/pd/utils/tcam:tcam_includes",
            "//nic/hal/pd/utils/acl_tcam:acl_tcam_includes",
            "//nic/hal/pd/utils/flow:flow_includes",
            "//nic/hal/pd/utils/hash:hash_includes",
            "//nic:gen_includes",
            "//nic/utils/bitmap",
            "//nic/utils/bm_allocator:bm_allocator",
            ],
)
