package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
cc_library(
    name = "sdklinkmgr",
    srcs = glob(["*.cc"]) + glob(["*.hpp"]) + glob(["*.h"]),
    hdrs = [
        "linkmgr.hpp",
    ],
    deps = [
        "//:sdk_headers",
        "//lib/pal:pal",
        "//lib/thread:thread",
        "//lib/timerfd:timerfd",
        "//lib/twheel:twheel",
        "//lib/catalog:catalog",
        "//:csr_includes",
        "@third_party//:aapl_includes",
    ],
)
