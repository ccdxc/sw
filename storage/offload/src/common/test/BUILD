# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_test(
    name = "pnso_logger_test",
    srcs = [
        "pnso_logger_test.c",
    ],
    copts = [
        "-g",
    ],
    deps = [
        "//storage/offload:pnso_logger",
        "//storage/offload:osal",
    ],
    linkopts = [
	"-lpthread"
    ],
)
