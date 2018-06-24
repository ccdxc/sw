package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license
#
# TODO: For now calling it as pnso_logger library.  Will change it to a
# generic utility library, if/as needed.
#
cc_library(
    name = "pnso_logger",
    srcs = [
        "src/drv/pnso_logger.c",
        "src/drv/pnso_logger.h",
        "src/drv/pnso_global.h",
        "src/drv/pnso_types.h",
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
    hdrs = glob(["src/osal/*.h"]) + [
        "include/pnso_api.h",
    ],
    alwayslink = 1
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

