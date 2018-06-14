package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "libpcieport",
    srcs = [
        "pcieport.c",
        "events.c",
        "ctrl.c",
        "intr.c",
        "fsm.c",
        "utils.c",
        "portcfg.c",
    ],
    hdrs = [
        "events.h",
        "pcieport_impl.h",
    ],
    copts = [ "-g",
              "-Iplatform/src/lib/misc/include",
              "-Iplatform/src/lib/pciehost/include",
              "-Iplatform/src/lib/pcieport/include",
              "-Iplatform/src/lib/pal/include",
              "-Iplatform/include/common",
              "-Inic/asic/capri/design/common",
              "-Inic/asic/capri/model/cap_he/readonly",
              "-Inic/asic/capri/model/cap_top/csr_defines",
              "-Inic/asic/capri/model/cap_txs",
              "-Inic/asic/capri/model/cap_top",
    ],
    deps = [
        "//platform/src/lib/misc/include:libmisc_hdrs",
        "//platform/src/lib/pciehost/include:libpciehost_hdrs",
        "//platform/src/lib/pcieport/include:libpcieport_hdrs",
        "//platform/src/lib/pal/include:libpal_hdrs",
        "//platform/include/common:platformcommon_hdrs",
        "//nic:platform_asic_hdrs",
    ],
    alwayslink = True,
)

