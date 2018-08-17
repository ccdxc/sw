package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "libpciehost",
    srcs = [
        "pciehdev.c",
        "pciehdev_bridgedn.c",
        "pciehdev_bridgeup.c",
        "pciehcfg.c",
        "pciehcfg_reset.c",
        "pciehbar.c",
        "pciehsys_linuxuser.c",
    ],
    hdrs = [
        "pciehbar_impl.h",
        "pciehcfg_impl.h",
        "pciehdev_impl.h",
        "pciehmgr_impl.h",
    ],
    copts = [ "-g",
              "-Iplatform/src/lib/misc/include",
              "-Iplatform/src/lib/cfgspace/include",
              "-Iplatform/src/lib/pciehost/include",
              "-Iplatform/src/lib/pciehw/include",
              "-Iplatform/src/lib/pciehdevices/include",
              "-Iplatform/include/common",
    ],
    deps = [
        "//platform/src/lib/misc/include:libmisc_hdrs",
        "//platform/src/lib/cfgspace/include:libcfgspace_hdrs",
        "//platform/src/lib/pciehost/include:libpciehost_hdrs",
        "//platform/src/lib/pciehw/include:libpciehw_hdrs",
        "//platform/src/lib/pciehdevices/include:libpciehdevices_hdrs",
        "//platform/include/common:platformcommon_hdrs",
    ],
    alwayslink = True,
)

