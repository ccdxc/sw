package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "wringpd",
    srcs = [] + glob(["*.cc"]),
    hdrs = [] + glob(["*.hpp"]) + glob(["*.h"]),
    copts = sdk_copts,
    deps = [
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic/hal:hal_hdrs",
        "//nic/hal/pd:pdcommon",
        "//nic:asic_includes",
        "//nic/hal/pd/capri:capri_includes",
        "//nic/hal/pd/iris:iris_includes",
        "//nic/utils/block_list:block_list",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/hal/periodic:periodic",
        "//nic/utils/bm_allocator",
        "//nic:gen_proto_includes",
        "//nic:google_includes",
        "//nic:grpc_includes",
    ],
    alwayslink = 1,
)

cc_library(
    name = "wringpd_includes",
    srcs = [],
    hdrs = ["wring_pd.hpp",
            ],
    deps = [
        "//nic:gen_includes",
        "//nic/hal:hal_hdrs",
        "//nic/hal/periodic:periodic",
    ],
)

