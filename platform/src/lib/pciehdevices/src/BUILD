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
              "-Iplatform/src/lib/pciehost/include",
              "-Iplatform/src/lib/pciehdevices/include",
              "-Iplatform/include/common",
              "-Inic/include",
              "-Inic/asic/capri/design/common",
              "-Inic/asic/capri/model/cap_he/readonly",
              "-Inic/asic/capri/model/cap_top/csr_defines",
              "-Inic/asic/capri/model/cap_txs",
              "-Inic/asic/capri/model/cap_top",
    ],
    deps = [
        "//platform/src/lib/pciehost/include:libpciehost_hdrs",
        "//platform/src/lib/pciehdevices/include:libpciehdevices_hdrs",
        "//platform/include/common:platformcommon_hdrs",
        "//nic/include:base_includes",
        "//nic:platform_asic_hdrs",
    ],
    alwayslink = True,
)

