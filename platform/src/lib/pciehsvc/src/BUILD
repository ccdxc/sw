package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "libpciehsvc",
    srcs = [
        "pciehsvc.c",
    ],
    hdrs = [
        "pciehsvc_impl.h",
    ],
    copts = [ "-g",
              "-Iplatform/src/lib/pciehsvc/include",
              "-Iplatform/src/lib/pciehost/include",
              "-Iplatform/src/lib/pciehw/include",
    ],
    deps = [
        "//platform/src/lib/pciehost/include:libpciehost_hdrs",
        "//platform/src/lib/pciehw/include:libpciehw_hdrs",
        "//platform/src/lib/pciehsvc/include:libpciehsvc_hdrs",
    ],
    alwayslink = True,
)

