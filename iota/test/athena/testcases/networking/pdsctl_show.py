#!/usr/bin/python3
import iota.harness.api as api
import iota.test.athena.utils.pdsctl as pdsctl
from iota.harness.infra.glopts import GlobalOptions

# Testing all pdsctl cmd
#
# For each NaplesHost
# pdsctl show port transceiver
# pdsctl show system statistics drop
# pdsctl show system --power
# pdsctl show system --temperature
# pdsctl show system statistics packet-buffer
# pdsctl show interrupts

def Setup(tc):
    api.Logger.info("Testing pdsctl show commands")
    return api.types.status.SUCCESS

def runPdsctlCmd(naples_nodes, cmd):
    if GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    for node in naples_nodes:
        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, cmd, yaml=False)
        if ret != True:
            api.Logger.error("show %s cmd cmd failed at node %s : %s" %(cmd, node, resp))
            return api.types.status.FAILURE    
    return api.types.status.SUCCESS

def Trigger(tc):
    naples_nodes = api.GetNaplesHostnames()
    cmd_list = ['port transceiver', 'system statistics drop', 'system --power', 'system --temperature', 'system statistics packet-buffer', 'interrupts']
    for cmd in cmd_list:
        ret = runPdsctlCmd(naples_nodes, cmd)
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Verify(tc):
    return api.types.status.SUCCESS


def Teardown(tc):
    return api.types.status.SUCCESS