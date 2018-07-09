package(default_visibility = ["//visibility:public"])

licenses(["notice"])  # MIT license

cc_library(
    name = "pnso_sim",
    srcs = glob(["*.c"]),
    deps = [
        "//storage/offload/include:pnso",
        "//storage/offload/src/osal:osal",
        "//storage/offload/third-party/freebsd-crypto:sim_crypto"
    ],
    copts = [
        "-xc"
    ],
    includes = [ "." ],
    hdrs = glob(["*.h"]),
    alwayslink = 1
)
