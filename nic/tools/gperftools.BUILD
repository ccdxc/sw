package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "gperftools",
    srcs = glob(["lib/libtcmalloc.so"]),
    hdrs = glob(["include/**/*.h"]),
    includes = ["include"],
)
