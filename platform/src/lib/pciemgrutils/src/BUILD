package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "libpciehost",
    srcs = [
        "pciehdev.c",
        "pciehcfg.c",
        "pciehbar.c",
    ],
    hdrs = [
    ],
    copts = [
        "-g",
        "-Iplatform/src/lib/misc/include",
        "-Iplatform/src/lib/cfgspace/include",
        "-Iplatform/src/lib/pciesys/include",
        "-Iplatform/src/lib/pciehost/include",
        "-Iplatform/src/lib/pciehdevices/include",
        "-Iplatform/src/lib/pcieport/include",
        "-Iplatform/include/common",
    ],
    deps = [
        "//platform/src/lib/misc/include:libmisc_hdrs",
        "//platform/src/lib/cfgspace/include:libcfgspace_hdrs",
        "//platform/src/lib/pciesys/include:libpciesys_hdrs",
        "//platform/src/lib/pciehost/include:libpciehost_hdrs",
        "//platform/src/lib/pciehw/include:libpciehw_hdrs",
        "//platform/src/lib/pciehdevices/include:libpciehdevices_hdrs",
        "//platform/src/lib/pcieport/include:libpcieport_hdrs",
        "//platform/include/common:platformcommon_hdrs",
    ],
    alwayslink = True,
)

