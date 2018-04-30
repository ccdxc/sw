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
        "src/sim"
    ],
    deps = [ "//nic:openssl-native" ],
    copts = [
        "-xc"
    ],
    hdrs = [
        "include/pnso_sim_api.h",
        "include/pnso_wafl.h"
    ],
)

cc_library(
    name = "spdk",
    deps = ["//storage/offload/third-party:spdk"],
)

cc_binary(
    name = "pnso_offloader",
    srcs = ["src/linux/uspace/offloader.c"],
    deps = ["//storage/offload:spdk"],
		copts = [ "-g" ],
    linkopts = [ "-lpthread", "-lrt", "-ldl", "-lnuma", "-luuid"],
)
