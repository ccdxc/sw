# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "asicpd",
    srcs = [
        "asic_pd_common.cc",
    ],
    hdrs = glob([
        "*.hpp",
        "*.h",
    ]),
    deps = [
        "//nic/include:base_includes",
        "//nic/hal/pd:hal_pd_includes",
        "//nic:gen_includes",
        "//nic:boost",
        "//nic:spdlog",
        "//nic:asic_includes",
        # TODO: Remove capri references
        "//nic/hal/pd/capri:capri_includes",
        "//nic/p4/nw/include:p4_nw_includes",
    ],
    alwayslink = 1,
)
