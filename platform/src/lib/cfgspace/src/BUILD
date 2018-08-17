package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "libcfgspace",
    srcs = [
        "cfgspace_getset.c",
        "cfgspace_header.c",
        "cfgspace_caps.c",
        "cfgspace_extcaps.c",
        "cfgspace_readwrite.c",
    ],
    hdrs = [
        "cfgspace_getset.h",
    ],
    copts = [ "-g",
              "-Iplatform/src/lib/cfgspace/include",
    ],
    deps = [
        "//platform/src/lib/cfgspace/include:libcfgspace_hdrs",
    ],
    alwayslink = True,
)

