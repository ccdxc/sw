# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_test(
    name = "hbm_hash_test",
    srcs = glob(["*.cc"]) + glob(["*.hpp"]),
    deps = [
            "//:sdk_headers",
            "//lib/table/hbmhash",
            "//lib/indexer:indexer",
            "//lib/logger",
            "//test/lib/table/p4pd_mock",
            "//lib/utils",
            "@third_party//:boost",
            "//:gtest",
    ],
    linkopts = [
        "-lrt",
    ]
)
