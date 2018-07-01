# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_test(
    name = "pnso_cp_ut",
    srcs = [
        "pnso_cp_ut.cc",
    ],
    copts = [
        "-g",
    ],
    deps = [
        "//storage/offload:pnso_drv",
        "//storage/offload/src/osal:osal",
        "//:gtest",
    ]
)
