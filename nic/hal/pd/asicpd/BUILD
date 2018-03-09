# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "asicpd",
    srcs = [] + glob(["*.cc"]),
    hdrs = [] + glob(["*.hpp"]),
    deps = [
        "//nic/include:base_includes",
        "//nic/hal/pd:hal_pd_includes",
        "//nic:gen_includes",
        "//nic:boost",
        "//nic:spdlog",
        "//nic:asic_includes",
        # TODO: Remove capri references
        "//nic/hal/pd/capri:capri_includes",
        "//nic/hal:hal_utils_includes",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/p4/iris/include:p4_iris_includes",
    ],
    alwayslink = 1,
)
