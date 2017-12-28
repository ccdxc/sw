package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "pal",
    srcs = [
        "pal.cc",
        "pal_sim.cc",
        "pal_hw.cc",
    ],
    hdrs = [
        "pal.hpp",
        "pal_internal.hpp"
    ],
    deps = [
        "//:sdk_headers",
    ],
)
