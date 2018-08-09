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
        "serdes.c",
        "cap_pcie_api.c",
        "cap_sbus_api.c",
        #"sbus_pcie_rom.s",
    ],
    data = [
        #"sbus_pcie_rom.bin",
    ],
    hdrs = [
        "events.h",
        "pcieport_impl.h",
        "cap_sw_glue.h",
        "cap_base_api.h",
        "cap_pcie_api.h",
        "cap_sbus_api.h",
    ],
    copts = [
        "-g",
        "-DCAPRI_SW",
        "-Iplatform/src/lib/misc/include",
        "-Iplatform/src/lib/pciehost/include",
        "-Iplatform/src/lib/pcieport/include",
        "-Iplatform/src/lib/pal/include",
        "-Iplatform/include/common",
        "-Inic/asic/capri/design/common",
        "-Inic/asic/capri/model/cap_top",
        "-Inic/asic/capri/model/cap_top/csr_defines",
        #"-Ibazel-out/k8-fastbuild/genfiles/platform/src/lib/pcieport/src",
        #"-I$(location sbus_pcie_rom.bin)",
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

genrule(
    name = "gen_sbus_pcie_rom.bin",
    srcs = [ "serdes.0x108C_2347.rom" ],
    outs = [ "sbus_pcie_rom.bin" ],
    cmd = "./$(location sbus_rom2bin.py) $(location serdes.0x108C_2347.rom) $@",
    tools = [ "sbus_rom2bin.py" ],
)
