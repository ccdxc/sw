#!/usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.utils.pdsctl as pdsctl

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

    return api.types.status.SUCCESS


def __getLifInfo(host_name):
    retval, output = pdsctl.ExecutePdsctlShowCommand(host_name, 'lif', yaml=False)

    if not retval:
        api.Logger.error("Failed to get lif info from pdsctl")
        output = ''

    return output


def Trigger(tc):
    fail = 0

    pairs = api.GetRemoteWorkloadPairs()
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
