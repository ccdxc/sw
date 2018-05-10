package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pnso_sim",
    srcs = glob(["src/sim/*.h"]) +
        glob(["src/sim/*.c"]) + [
        "src/osal/pnso_sim_osal.h",
        "src/osal/linux/uspace/pnso_sim_uspace.c",
    ],
    includes = [
        "include",
        "src/sim",
        "src/osal"
    ],
    deps = [ "//nic:openssl-native" ],
    copts = [
        "-xc"
    ],
    hdrs = [
        "include/pnso_api.h",
        "include/pnso_wafl.h",
        "src/sim/pnso_sim.h"
    ],
    alwayslink = 1
)

cc_library(
    name = "spdk",
    deps = ["//storage/offload/third-party:spdk"],
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
	"//storage/offload:spdk",
	"pnso_sim",
	],
    copts = [ "-g" ],
    linkopts = [ "-lpthread", "-lrt", "-ldl", "-lnuma", "-luuid"],
)
