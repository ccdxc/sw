package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "libpal",
    srcs = [
        "pal.c",
        "reg.c",
        "mem.c",
        "mmap_region.c",
        "trace.c",
        "pal_mm.c",
    ],
    hdrs = [
        "pal_impl.h",
        "pal_mm.h",
    ],
    copts = [ "-g",
              "-Iplatform/src/lib/pal/include",
              "-Inic/asic/capri/design/common",
              "-Inic/asic/capri/model/cap_he/readonly",
              "-Inic/asic/capri/model/cap_top/csr_defines",
              "-Inic/asic/capri/model/cap_txs",
              "-Inic/asic/capri/model/cap_top",
    ],
    deps = [
        "//platform/src/lib/pal/include:libpal_hdrs",
        "//nic:platform_asic_hdrs",
    ],
    alwayslink = True,
)

