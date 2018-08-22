# {C} Copyright 2017 Pensando Systems Inc. All rights reserved
package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "periodic_src",
    srcs = glob(["*.cc"]),
    hdrs = glob(["*.hpp"]),
    deps = [
        "//:sdk_headers",
        "//lib/twheel:twheel",
        "//lib/timerfd:timerfd",
        "//lib/slab:slab",
        "//lib/shmmgr:shmmgr",
    ],
)

cc_binary(
    name = "libperiodic.so",
    deps = [
        ":periodic_src",
    ],
    linkshared = 1,
)
