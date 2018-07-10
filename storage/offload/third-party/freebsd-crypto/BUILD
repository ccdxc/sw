package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "sim_crypto",
    srcs = glob(["src/*.c"]),
    hdrs = glob(["src/*.h"]),
    includes = [ "src" ],
    deps = [ "//storage/offload/src/osal:osal" ],
    alwayslink = 1,
)
