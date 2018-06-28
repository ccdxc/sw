# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pnso_logger",
    srcs = [
        "pnso_logger.c",
        "pnso_global.h",
    ],
    includes = [
        "include",
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
        "pnso_logger.h",
    ],
    alwayslink = 1
)
