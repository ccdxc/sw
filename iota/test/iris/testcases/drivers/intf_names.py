#!/usr/bin/python3
import iota.harness.api as api
import iota.test.utils.naples_host as naples_host
import iota.test.utils.ionic_utils as ionic_utils
# Testing that interface names get pushed to Naples
#
# For each NaplesHost
#   get halctl show lifs
#   For each interface
#     make sure it appears in show_lifs output
#

def Setup(tc):
    api.Logger.info("Interface Names")
    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    if tc.os == naples_host.OS_TYPE_ESX:
        api.Logger.info("Not implemented for %s" % tc.os)
        return api.types.status.IGNORED

    return api.types.status.SUCCESS


def __getLifInfo(host_name):
    req = api.Trigger_CreateExecuteCommandsRequest(serial=True)

    api.Trigger_AddNaplesCommand(req, host_name, "/nic/bin/halctl show lif")
    resp = api.Trigger(req)

    cmd = resp.commands[0]
    api.PrintCommandResults(cmd)

    if cmd.exit_code != 0:
        api.Logger.error("Bad exit code %d on naples %s" % (cmd.exit_code, host_name))

    return cmd.stdout


def Trigger(tc):
    if tc.os == naples_host.OS_TYPE_ESX:
        api.Logger.info("Not implemented for %s" % tc.os)
        return api.types.status.IGNORED
    
    fail = 0   
    pairs = api.GetLocalWorkloadPairs()
    hosts = pairs[0]

    for host in hosts:
        if not host.IsNaples():
            continue

        api.Logger.info("Checking host %s" % host.node_name)
        lif_info = __getLifInfo(host.node_name)

        intfs = api.GetNaplesHostInterfaces(host.node_name)
        for intf in intfs:
            # Windows HAL interface name is translated two times.
            if tc.os == naples_host.OS_TYPE_WINDOWS:
                intf = ionic_utils.winHalIntfName(host.node_name, intf)
                # HAL yaml o/p is Pen..Adapter, halctl cli op/ is pen..adap.
                intf = intf.lower();

            if lif_info.find(intf) == -1:
                api.Logger.error("interface %s not found" % intf)
                fail += 1

    if fail != 0:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Verify(tc):
    return api.types.status.SUCCESS


def Teardown(tc):
    return api.types.status.SUCCESS
