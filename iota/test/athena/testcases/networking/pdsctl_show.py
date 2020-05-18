#!/usr/bin/python3
import iota.harness.api as api
import iota.harness.infra.store as store
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
    tc.nics =  store.GetTopology().GetNicsByPipeline("athena")
    tc.nodes= []
    for nic in tc.nics:
        tc.nodes.append(nic.GetNodeName())
    api.Logger.info("Testing pdsctl show commands on nodes {}".format(tc.nodes))
    return api.types.status.SUCCESS

def runPdsctlCmd(tc, cmd):
    if GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    for node in tc.nodes:
        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, cmd, yaml=False)
        if ret != True:
            api.Logger.error("show %s cmd cmd failed at node %s : %s" %(cmd, node, resp))
            return api.types.status.FAILURE    
    return api.types.status.SUCCESS

def Trigger(tc):
    cmd_list = ['port transceiver', 'system statistics drop', 'system --power', 'system --temperature', 'system statistics packet-buffer', 'interrupts']
    for cmd in cmd_list:
        ret = runPdsctlCmd(tc, cmd)
        if ret != api.types.status.SUCCESS:
            return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Verify(tc):
    return api.types.status.SUCCESS


def Teardown(tc):
    return api.types.status.SUCCESS