#!/usr/bin/python3
import iota.harness.api as api
import iota.test.utils.naples_host as naples_host

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

    if tc.os != naples_host.OS_TYPE_BSD and tc.os != naples_host.OS_TYPE_LINUX:
        api.Logger.info("Not implemented for %s" % tc.os)
        return api.types.status.IGNORED

    pairs = api.GetLocalWorkloadPairs()
    if not pairs:
        api.Logger.error("No workloads found")
        return api.types.status.FAILURE

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
    if tc.os != naples_host.OS_TYPE_BSD and tc.os != naples_host.OS_TYPE_LINUX:
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
            api.Logger.info("Checking interface %s" % intf)

            if lif_info.find(intf) == -1:
                api.Logger.info("interface %s not found" % intf)
                fail += 1

    if fail != 0:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Verify(tc):
    return api.types.status.SUCCESS


def Teardown(tc):
    return api.types.status.SUCCESS
