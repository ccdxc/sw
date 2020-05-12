#! /usr/bin/python3
import time
import json
import tarfile
import iota.harness.api as api
import iota.test.iris.testcases.penctl.common as common
import iota.test.iris.testcases.server.verify_pci as verify_pci

def getdeviceid(device, node):
    get_deviceid_cmd = f"setpci -s {device} DEVICE_ID"
    resp = verify_pci.run_command(get_deviceid_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code == 0:
            return int(cmd.stdout)

    api.Logger.error("Unable to get device id for device ", device)
    return -1

def getsubsystemid(device, deviceid, node):
    if deviceid == 1000 or deviceid == 1001:
        get_subsystemid_cmd = f"setpci -s {device} CAP_SSVID+6.w"
    else:
        get_subsystemid_cmd = f"setpci -s {device} SUBSYSTEM_ID"
    resp = verify_pci.run_command(get_subsystemid_cmd, node)
    if resp:
        cmd = resp.commands.pop()
        if cmd.exit_code == 0:
            return int(cmd.stdout)

    api.Logger.error("Unable to get subsystem id for device ", device)
    return -1

def checkpcidevice(node, expected_subsystemid):
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
        deviceid = getdeviceid(device, node)
        subsystemid = getsubsystemid(device, deviceid, node)
        if subsystemid != int(expected_subsystemid):
            api.Logger.error("Subsystem id didnt match for ", device, node)
            return api.types.status.CRITICAL
    return api.types.status.SUCCESS

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("Starting PCIe Device ID test")
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

        expected_subsystemid = verify_pci.pci_partnum_list[naples_partnum]["subsystem_id"]
        if (checkpcidevice(node, expected_subsystemid) == api.types.status.CRITICAL):
            return api.types.status.CRITICAL
    api.Logger.info("PCI subsystem device id matches for all systems")
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
