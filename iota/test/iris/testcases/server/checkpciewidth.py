#! /usr/bin/python3
import time
import json
import tarfile
import re
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common
import iota.test.iris.testcases.server.verify_pci as verify_pci
import iota.test.iris.testcases.server.checkpcieid as checkpcieid

def checkpciewidth(node, expected_width, pcielinklist):
    cap_width = 0
    sta_width = 0
    if len(pcielinklist) == 2:
        cap_width = re.compile('Width x([0-9]+),').findall(pcielinklist[0])
        sta_width = re.compile('Width x([0-9]+),').findall(pcielinklist[0])
    else:
        api.Logger.error("Unable to get the widths", pcielinklist)
        return api.types.status.CRITICAL

    if int(expected_width) == int(cap_width[0]) and int(cap_width[0]) == int(sta_width[0]):
        return api.types.status.SUCCESS

    api.Logger.error("Widths dont match")
    api.Logger.error("expected", expected_width, "cap", cap_width, "sta", sta_width)
    return api.types.status.CRITICAL

def verifypciwidth(node, expected_width):
    pcidevlist = []
    pci_getdev_cmd = f"lspci -Dnd 1dd8: | awk ' {{print $1}}'"
    resp = verify_pci.run_command(pci_getdev_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code != 0:
            api.Logger.error("Unable to device list with 1dd8 vendor id")
            return api.types.status.CRITICAL
        pcidevlist = cmd.stdout.splitlines()

    for device in pcidevlist:
        deviceid = checkpcieid.getdeviceid(device, node)
        # Skip root bridge
        if deviceid == 1000:
            continue
        pci_verif_cmd = f"lspci -nnvvv -s {device} | grep 'LnkCap:\|LnkSta:'"
        resp = verify_pci.run_command(pci_verif_cmd, node)
        if resp:
            cmd = resp.commands.pop()
            if cmd.exit_code != 0:
                api.Logger.error("Error getting width capability")
                return api.types.status.CRITICAL
        linklist = cmd.stdout.splitlines()
        if (checkpciewidth(node, expected_width, linklist) != api.types.status.SUCCESS):
            api.Logger.error("Unable to verify width")
            return api.types.status.CRITICAL

    return api.types.status.SUCCESS

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Starting PCIe width test")
    tc.Nodes = api.GetNaplesHostnames()

    for node in tc.Nodes:
        naples_fru = common.GetNaplesFruJson(node)
        # Find the naples part number
        naples_partnum = naples_fru["part-number"]
        if naples_partnum == None:
            api.Logger.error("Cannot find naples part number")
            return api.types.status.FAILURE

        if naples_partnum.startswith("68-"):
            naples_partnum = naples_partnum[0:6]

        expected_width = verify_pci.pci_partnum_list[naples_partnum]["width"]
        if (verifypciwidth(node, expected_width) == api.types.status.CRITICAL):
            return api.types.status.CRITICAL
    api.Logger.info("PCI subsystem device id matches for all systems")
    return api.types.status.SUCCESS


def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS

