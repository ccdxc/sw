# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_test(
    name = "pnso_pbuf_ut",
    srcs = [
        "pnso_pbuf_ut.cc",
    ],
    copts = [
        "-g",
    ],
    deps = [
        "//storage/offload/src/common:pnso_pbuf",
        "//storage/offload/src/osal:osal",
        "//:gtest",
    ]
)
