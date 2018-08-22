package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "libpciehdevices",
    srcs = [
        "accel.c",
        "debug.c",
        "eth.c",
        "nvme.c",
        "pciestress.c",
        "rcdev.c",
        #"utils.c",
        "virtio.c",
    ],
    hdrs = [
        "utils_impl.h",
    ],
    copts = [ "-g",
              "-Iplatform/src/lib/misc/include",
              "-Iplatform/src/lib/pciehost/include",
              "-Iplatform/src/lib/pciehdevices/include",
              "-Iplatform/include/common",
              "-Inic/include",
    ],
    deps = [
        "//platform/src/lib/misc/include:libmisc_hdrs",
        "//platform/src/lib/pciehost/include:libpciehost_hdrs",
        "//platform/src/lib/pciehdevices/include:libpciehdevices_hdrs",
        "//platform/include/common:platformcommon_hdrs",
        "//nic/include:base_includes",
    ],
    alwayslink = True,
)

