package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "libintrutils",
    srcs = [
        "intrutils.c",
    ],
    hdrs = [
    ],
    copts = [ "-g",
              "-Iplatform/src/lib/intrutils/include",
              "-Iplatform/src/lib/pal/include",
              "-Inic/asic/capri/design/common",
              "-Inic/asic/capri/model/cap_he/readonly",
              "-Inic/asic/capri/model/cap_top/csr_defines",
              "-Inic/asic/capri/model/cap_txs",
              "-Inic/asic/capri/model/cap_top",
    ],
    deps = [
        "//platform/src/lib/intrutils/include:libintrutils_hdrs",
        "//platform/src/lib/pal/include:libpal_hdrs",
        "//nic:platform_asic_hdrs",
    ],
    alwayslink = True,
)

