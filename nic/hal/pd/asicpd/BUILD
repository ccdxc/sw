# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "asicpd",
    srcs = [] + glob(["*.cc"]),
    hdrs = [] + glob(["*.hpp"]),
    deps = [
        "//nic/hal/pd:hal_pd_includes",
        "//nic:gen_common_includes",
        "//nic:boost",
        "//nic:spdlog",
        "//nic:asic_includes",
        # TODO: Remove capri references
        "//nic/hal/pd/capri:capri_includes",
        "//nic/hal:hal_utils_includes",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/hal/iris/datapath/p4/include:p4_iris_includes",
        "//nic/p4/common:p4_common_includes",
    ],
    alwayslink = 1,
)
