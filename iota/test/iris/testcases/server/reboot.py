import iota.harness.api as api
import iota.test.utils.naples_host as host
import iota.test.iris.testcases.server.verify_pci as verify_pci
from iota.harness.infra.exceptions import *

def Setup (tc):
    api.Logger.info("Server Compatiblity Reboot: APC, IPMI, OS")

    tc.nodes = api.GetNaplesHostnames()
    tc.os = api.GetNodeOs(tc.nodes[0])

    if len(tc.nodes) > 1:
        api.Logger.info(f"Expecting one node setup, this testbed has {len(tc.nodes)}")
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Trigger (tc):
    naples_nodes = []
    #for every node in the setup
    for node in tc.nodes:
        if api.IsNaplesNode(node):
            naples_nodes.append(node)
            api.Logger.info(f"Found Naples Node: [{node}]")

    if len(naples_nodes) == 0:
        api.Logger.error(f"Failed to find a Naples Node!")
        return api.types.status.FAILURE

    for reboot in range(tc.args.reboots):
        # Reboot Node.
        # Reboot method (APC, IPMI, OS Reboot) is passed as a testcase parameter
        for node in naples_nodes:
            api.Logger.info(f"Starting Reboot Loop # {reboot} on {node}. Reboot method: {tc.iterators.reboot_method}")
            if api.RestartNodes([node], tc.iterators.reboot_method) == api.types.status.FAILURE:
                raise OfflineTestbedException

            api.Logger.info(f"Verifying PCI on [{node}]: ")
            if verify_pci.verify_errors_lspci(node, tc.os) != api.types.status.SUCCESS:
                api.Logger.error(f"PCIe Failure detected on {node}")
                raise OfflineTestbedException

    return api.types.status.SUCCESS

def Verify (tc):

    return api.types.status.SUCCESS
