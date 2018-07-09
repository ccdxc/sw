# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pnso_pbuf",
    srcs = [
        "pnso_pbuf.c",
    ],
    includes = [
        "src/common",
    ],
    deps = [
	"//storage/offload/src/osal:osal",
	"//storage/offload/include:pnso",
    ],
    copts = [
        "-g",
        "-xc",
    ],
    hdrs = [
        "pnso_pbuf.h",
    ],
    alwayslink = 1
)
