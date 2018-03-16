# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
load("//nic/bazel:common_deps.bzl", "sdk_copts")

cc_library(
    name = "hal_pd_includes",
    srcs = [],
    hdrs = [
        "p4pd_api.hpp",
        "hal_pd_error.hpp",
        "asic_pd.hpp",
        "hal_pd.hpp",
        "cpupkt_api.hpp",
        "cpupkt_headers.hpp",
        "pd_api.hpp",
        "pd_api_c.h"
    ],
    deps = [
        "//nic/hal:hal_includes",
    ]
)

cc_library(
    name = "pdcommon",
    srcs = [
        "asic_pd.cc",
        "hal_pd.cc",
        "pd.hpp",
        "pd_api.hpp",
        "pd_api_c.h",
    ],
    hdrs = [
        "cpupkt_api.hpp",
        "cpupkt_headers.hpp",
        "pd.hpp",
        "pd_api.hpp",
        "pd_api_c.h",
    ],
    copts = sdk_copts,
    deps = [
        "@sdk//obj:sdk_ht",
        "@sdk//obj:sdk_pal",
        "@sdk//obj:sdk_list",
        "@sdk//obj:sdk_thread",
        "//nic/utils/block_list:block_list",
        "//nic/hal/periodic:periodic",
        "//nic:gen_proto_includes",
        "//nic/hal/pd:hal_pd_includes",
        "//nic/hal/lib:hal_lib_includes",
        "//nic/include:base_includes",
        "//nic:spdlog",
        "//nic:boost",
        "//nic:google_includes",
        "//nic:grpc_includes",
        "//nic/utils/bm_allocator",
    ],
    alwayslink = 1,
)
