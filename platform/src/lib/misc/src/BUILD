package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "libmisc",
    srcs = [
        "hex_format.c",
        "maclib.c",
        "mapfile.c",
        "strncpy0.c",
        "strtoargv.c",
        "strtoull_ext.c",
        "human_readable.c",
    ],
    hdrs = [
    ],
    copts = [ "-g",
              "-Iplatform/src/lib/misc/include",
    ],
    deps = [
        "//platform/src/lib/misc/include:libmisc_hdrs",
    ],
    alwayslink = True,
)

