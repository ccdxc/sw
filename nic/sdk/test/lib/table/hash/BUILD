# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

# target: hash test
cc_test(
    name = "hash_test",
    srcs = [
        "hash_test.cc",
    ],
    deps = [
        "//:sdk_headers",
        "//lib/table/hash",
        "//lib/table/tcam:tcam",
        "//lib/logger",
        "//test/lib/table/p4pd_mock",
        "//lib/indexer:indexer",
        "//lib/ht:ht",
        # External
        "//:gtest",
    ],
)
