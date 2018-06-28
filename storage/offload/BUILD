# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pnso_logger",
    deps = [
        "//storage/offload/src/common:pnso_logger",
    ],
)

cc_library(
    name = "osal",
    deps = [
        "//storage/offload/src/osal:osal",
    ],
)

cc_binary(
    name = "pnso_offloader",
    srcs = [
        "src/linux/uspace/offloader.c",
        "src/linux/uspace/offloader.h",
        "src/linux/uspace/io_initiator.c",
        "src/linux/uspace/eff.c",
        "src/linux/uspace/xor.c",
        "src/linux/uspace/read.c",
    ],
    deps = [
        "//storage/offload/third-party:spdk",
        "//storage/offload/src/sim:pnso_sim",
    ],
    copts = [ "-g" ],
    linkopts = [ "-lpthread", "-lrt", "-ldl", "-lnuma", "-luuid"],
)
