#!/usr/bin/python3
import iota.harness.api as api
import iota.harness.infra.store as store
import iota.test.athena.utils.pdsctl as pdsctl
from iota.harness.infra.glopts import GlobalOptions
# from apollo.config.store import client as EzAccessStoreClient

# Testing all pdsctl port cmd
#
# For each NaplesHost
# pdsctl show port status
# pdsctl show port statistics 


def Setup(tc):
    tc.nics =  store.GetTopology().GetNicsByPipeline("athena")
    tc.nodes= []
    for nic in tc.nics:
        tc.nodes.append(nic.GetNodeName())
    api.Logger.info("Test PDSCTL for Athena pipeline on node {}".format(tc.nodes))
    return api.types.status.SUCCESS

def showPortStatusCmd(tc):
    if GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    for node in tc.nodes:
        # TODO: add EzAccessStoreClient support and un-comment following code
        # node_uuid = EzAccessStoreClient[node].GetNodeUuid(node)
        # for uplink in [UPLINK_PREFIX1, UPLINK_PREFIX2]:
        #     intf_uuid = uplink % node_uuid  
        #     cmd = "show port status -p "+intf_uuid
        #     ret, resp = pdsctl.ExecutePdsctlCommand(node, cmd, yaml=False)
        #     if ret != True:
        #         api.Logger.error("show port status -p cmd failed at node %s : %s" %(node, resp))
        #         return api.types.status.FAILURE      
        # misc_utils.Sleep(3)
        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, 'port status', yaml=False)
        if ret != True:
            api.Logger.error("show port status cmd failed at node %s : %s" %(node, resp))
            return api.types.status.FAILURE    
    return api.types.status.SUCCESS

def showPortStatisticsCmd(tc):
    if GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    for node in tc.nodes:
        # node_uuid = EzAccessStoreClient[node].GetNodeUuid(node)
        # for uplink in [UPLINK_PREFIX1, UPLINK_PREFIX2]:
        #     intf_uuid = uplink % node_uuid  
        #     cmd = "show port statistics -p "+intf_uuid
        #     ret, resp = pdsctl.ExecutePdsctlCommand(node, cmd, yaml=False)
        #     if ret != True:
        #         api.Logger.error("show port statistics -p cmd failed at node %s : %s" %(node, resp))
        #         return api.types.status.FAILURE      
        # misc_utils.Sleep(3)
        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, 'port statistics', yaml=False)
        if ret != True:
            api.Logger.error("show port statistics cmd failed at node %s : %s" %(node, resp))
            return api.types.status.FAILURE    
    return api.types.status.SUCCESS

def Trigger(tc):
    api.Logger.info("show port status on %s ..."%tc.nodes)
    ret = showPortStatusCmd(tc)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    api.Logger.info("show port statistics on %s ..."%tc.nodes)
    ret = showPortStatisticsCmd(tc)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Verify(tc):
    return api.types.status.SUCCESS


def Teardown(tc):
    return api.types.status.SUCCESS