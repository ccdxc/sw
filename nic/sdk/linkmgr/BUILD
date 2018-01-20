package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license
cc_library(
    name = "linkmgr",
    srcs = glob(["*.cc"]) + glob(["*.hpp"]),
    hdrs = [
        "linkmgr.hpp",
        ],
    deps = [
        "//:sdk_headers",
        "//lib/catalog:catalog",
    ],
)
