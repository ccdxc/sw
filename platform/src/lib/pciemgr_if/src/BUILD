package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "libpciemgr",
    srcs = [
        "pciemgr.cc",
    ],
    hdrs = [
    ],
    copts = [
        "-g",
        "-Iplatform/src/lib/pciemgr/include",
    ],
    deps = [
        "//platform/src/lib/pciemgr/include:libpciemgr_hdrs",
    ],
    alwayslink = True,
)

