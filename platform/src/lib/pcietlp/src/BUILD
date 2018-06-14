package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "libpcietlp",
    srcs = [
        "pcietlp.c",
    ],
    hdrs = [
    ],
    copts = [ "-g",
              "-Iplatform/src/lib/misc/include",
              "-Iplatform/src/lib/pcietlp/include",
    ],
    deps = [
        "//platform/src/lib/misc/include:libmisc_hdrs",
        "//platform/src/lib/pcietlp/include:libpcietlp_hdrs",
    ],
    alwayslink = True,
)

