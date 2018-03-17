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
        "virtio.c",
    ],
    copts = [
        "-g",
	"-Iplatform/include/common",
	"-Iplatform/src/lib/misc/include",
	"-Iplatform/src/lib/intrutils/include",
	"-Iplatform/src/lib/pciehost/include",
	"-Iplatform/src/lib/pciehdevices/include",
	"-Inic/include",
    ],
    deps = [
        "//platform/src/lib/misc/include:libmisc_hdrs",
        "//platform/src/lib/pciehdevices/include:libpciehdevices_hdrs",
        "//platform/src/lib/pciehost/include:libpciehost_hdrs",
        "//platform/src/lib/intrutils/include:libintrutils_hdrs",
        "//platform/include/common:platformcommon_hdrs",
        "//nic/include:base_includes",
    ],
    alwayslink = True,
)

