# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
#
# TODO: For now calling it as pnso_dpool library.  Will change it to a
# generic utility library, if/as needed.
#
cc_library(
    name = "pnso_dpool",
    srcs = [
        "src/drv/pnso_dpool.c",
    ],
    includes = [
        "include",
        "src/osal",
        "src/drv",
    ],
    deps = [
	"osal",
    ],
    copts = [
        "-g",
        "-Isrc/drv",
        "-xc",
    ],
    hdrs = [
        "include/pnso_api.h",
        "src/drv/pnso_dpool.h",
    ],
    alwayslink = 1
)

cc_library(
    name = "pnso_drv",
    deps = [
        "//storage/offload/src/drv:pnso_drv",
    ],
)

cc_library(
    name = "osal",
    deps = [
        "//storage/offload/src/osal:osal",
    ],
)

cc_library(
    name = "pnso_pbuf",
    deps = [
        "//storage/offload/src/common:pnso_pbuf",
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
