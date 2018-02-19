# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

# target: indexer test
cc_test(
    name = "tcam_test",
    srcs = [
        "tcam_test.cc",
    ],
    deps = [
        "//:sdk_headers",
        "//lib/table/tcam",
        "//lib/indexer:indexer",
        "//lib/ht:ht",
        "//lib/logger",
        "//test/lib/table/p4pd_mock",
        # External
        "//:gtest",
    ],
)
