package(default_visibility = ["//visibility:public"])
licenses(["notice"])  # MIT license

cc_library(
    name = "libpcieport",
    srcs = [
        "pcieport.c",
        "events.c",
        "config.c",
        "intr.c",
        "fsm.c",
        "utils.c",
        "portcfg.c",
        "serdes.c",
        "cap_pcie_api.c",
        "cap_sbus_api.c",
	# XXX figure out how to compile this!
        #"sbus_pcie_rom.s",
    ],
    data = [
        "sbus_pcie_rom.bin",
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
	"-Inic/asic/capri/design/common",
	"-Inic/asic/capri/model/cap_top",
	"-Inic/asic/capri/model/cap_top/csr_defines",
	"-Iplatform/include/common",
	"-Iplatform/src/lib/misc/include",
	"-Iplatform/src/lib/pciehost/include",
	"-Iplatform/src/lib/pciehw/include",
	"-Iplatform/src/lib/pciesys/include",
	"-Iplatform/src/lib/pal/include",
	"-Iplatform/src/lib/pcieport/include",
    ],
    deps = [
        "//platform/src/lib/misc/include:libmisc_hdrs",
        "//platform/src/lib/pciesys/include:libpciesys_hdrs",
        "//platform/src/lib/pal/include:libpal_hdrs",
        "//platform/src/lib/pcieport/include:libpcieport_hdrs",
        "//platform/src/lib/pciehost/include:libpciehost_hdrs",
        "//platform/src/lib/pciehw/include:libpciehw_hdrs",
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
