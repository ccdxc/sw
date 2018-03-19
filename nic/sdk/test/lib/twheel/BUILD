# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

# Slab Test
cc_test(
    name = "twheel_test",
    srcs = ["twheel_test.cc"],
    deps = [
        "//:gtest",
        "//:sdk_headers",
        "//lib/twheel:twheel",
        "//lib/slab:slab",
        "//lib/shmmgr:shmmgr",
        "//lib/logger:logger",
    ],
    linkopts = [
        "-lrt",
    ],
)
