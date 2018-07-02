# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

#
# TODO: For now calling it as pnso_pbuf library.  Will change it to a
# generic utility library, if/as needed.
#
cc_library(
    name = "osal",
    deps = [
        "//storage/offload/src/osal:osal",
    ],
)

cc_library(
    name = "pnso_pbuf",
    srcs = [
        "src/drv/pnso_pbuf.c",
        "src/drv/pnso_pbuf.h",
    ],
    includes = [
        "include",
        "src/osal",
    ],
    deps = [
	"osal",
    ],
    copts = [
        "-g",
    ],
    hdrs = [
        "include/pnso_api.h",
    ],
    alwayslink = 1
)

cc_binary(
    name = "pnso_pbuf_ut",
    srcs = [
        "src/drv/test/pnso_pbuf_ut.c",
    ],
    includes = [
        "include",
        "src/osal",
        "src/drv",
    ],
    deps = [
	"osal",
	"pnso_pbuf",
    ],
    copts = [
        "-g",
    ],
    linkopts = [
	"-lm",
	"-lz",
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
