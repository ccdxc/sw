# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

# target: directmap test
cc_test(
    name = "directmap_test",
    srcs = [
        "directmap_test.cc",
    ],
    deps = [
        "//:sdk_headers",
        "//lib/table/directmap",
        "//lib/logger",
        "//test/lib/table/p4pd_mock",
        # External
        "//:gtest",
    ],
)
