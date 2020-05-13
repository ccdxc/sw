#!/usr/bin/python3
import iota.harness.api as api
import iota.test.athena.utils.pdsctl as pdsctl
from iota.harness.infra.glopts import GlobalOptions
# from apollo.config.store import client as EzAccessStoreClient

# Testing all pdsctl port cmd
#
# For each NaplesHost
# pdsctl show port status
# pdsctl show port statistics 


def Setup(tc):
    api.Logger.info("Test PDSCTL for Athena pipeline")
    return api.types.status.SUCCESS

def showPortStatusCmd(naples_nodes):
    if GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    for node in naples_nodes:
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

def showPortStatisticsCmd(naples_nodes):
    if GlobalOptions.dryrun:
        return api.types.status.SUCCESS
    for node in naples_nodes:
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
    naples_nodes = api.GetNaplesHostnames()

    api.Logger.info("show port status on %s ..."%naples_nodes)
    ret = showPortStatusCmd(naples_nodes)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    api.Logger.info("show port statistics on %s ..."%naples_nodes)
    ret = showPortStatisticsCmd(naples_nodes)
    if ret != api.types.status.SUCCESS:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS


def Verify(tc):
    return api.types.status.SUCCESS


def Teardown(tc):
    return api.types.status.SUCCESS