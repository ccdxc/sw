# {C} Copyright 2017 Pensando Systems Inc. All rights reserved

package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_test(
    name = "shmmgr_test",
    srcs = ["shmmgr_test.cc"],
    deps = [
        "//:sdk_headers",
        "//obj:sdk_shmmgr",
        "//obj:sdk_logger",
        "//:gtest",
        "@third_party//:boost",
    ],
    linkopts = [
        "-lrt",
    ]
)
