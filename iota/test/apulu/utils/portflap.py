#! /usr/bin/python3
import iota.harness.api as api
from time import *
from apollo.config.store import client as EzAccessStoreClient
import iota.test.apulu.utils.pdsctl as pdsctl
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

UPLINK_PREFIX1 = "01000111-0000-0000-4242-00%x"
UPLINK_PREFIX2 = "01000211-0000-0000-4242-00%x"
operUp = "up"
operDown = "down"
adminUp = "up"
adminDown = "down"
verifyRetry = 120 #no of seconds to retry for


def verifyDataPortState(naples_nodes, admin, oper):
    ret = api.types.status.SUCCESS
    if GlobalOptions.dryrun:
        return ret
    retry_remaining = verifyRetry
    ret = verifyDataPortStateHelper(naples_nodes, admin, oper)
    while api.types.status.FAILURE == ret and retry_remaining > 0:
        sleep(1)
        retry_remaining = retry_remaining - 1
        ret = verifyDataPortStateHelper(naples_nodes, admin, oper)
    
    return ret

def verifyDataPortStateHelper(naples_nodes, admin, oper):
    ret = api.types.status.SUCCESS
    for node in naples_nodes:
        node_uuid = EzAccessStoreClient[node].GetNodeUuid(node)
        #if not node_uuid:
        #     node_uuid = int(''.join(filter(str.isdigit, node)))
        for uplink in [UPLINK_PREFIX1, UPLINK_PREFIX2]:
            intf_uuid = uplink % node_uuid 
            cmd = "port status -p "+intf_uuid
            ret, resp = pdsctl.ExecutePdsctlShowCommand(node, cmd, yaml=False)
            if ret != True:
                api.Logger.error("oper:%s uplink verify failed at node %s : %s" %(admin, node, resp))
                return api.types.status.FAILURE
            else :
                ret = api.types.status.SUCCESS #explicitly mark SUCCESS
            if oper == operUp:
                if "UP          UP" not in resp:
                    api.Logger.error("oper:%s uplink verify failed at node %s : %s" %(oper, node, resp))
                    return api.types.status.FAILURE
            elif admin == operDown:
                if "DOWN        DOWN" not in resp:
                    api.Logger.error("admin:%s uplink verify failed at node %s : %s" %(admin, node, resp))
                    return api.types.status.FAILURE
            else:
                if "UP          DOWN" not in resp:
                    api.Logger.error("admin:%s uplink verify failed at node %s : %s" %(admin, node, resp))
                    return api.types.status.FAILURE
    api.Logger.info("verifyDataPortState done for %s and result %d..." %(naples_nodes,ret))
    return ret
 
def switchPortFlap(tc):
    flap_count = 1
    num_ports = 1
    interval = 2
    down_time  = 2
    naples_nodes = api.GetNaplesHostnames()

    api.Logger.info("Flapping switch port on %s ..."%naples_nodes)
    ret = api.FlapDataPorts(naples_nodes, num_ports, down_time, flap_count, interval)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Failed to flap the switch port")
        return ret

    sleep(2) #give a short gap before printing status
    return api.types.status.SUCCESS

def setDataPortStatePerUplink(naples_nodes, oper, id):
    uplink_list = []
    if id == 0:
        uplink_list.append(UPLINK_PREFIX1)
    else :
        uplink_list.append(UPLINK_PREFIX2)

    for node in naples_nodes:
        node_uuid = EzAccessStoreClient[node].GetNodeUuid(node)
        #node_uuid = 750763714960
        for uplink in uplink_list:
            intf_uuid = uplink % node_uuid  
            cmd = ("debug port --admin-state %s --port "+intf_uuid) % oper
            ret, resp = pdsctl.ExecutePdsctlCommand(node, cmd, yaml=False)
            if ret != True:
                api.Logger.error("oper:%s uplink failed at node %s : %s" %(oper, node, resp))
                return api.types.status.FAILURE      
        sleep(1) #give a short gap before printing status
        pdsctl.ExecutePdsctlShowCommand(node, "port status", yaml=False)
    return api.types.status.SUCCESS

def setDataPortState(naples_nodes, oper):
    for node in naples_nodes:
        node_uuid = EzAccessStoreClient[node].GetNodeUuid(node)
        #node_uuid = 750763714960
        for uplink in [UPLINK_PREFIX1, UPLINK_PREFIX2]:
            intf_uuid = uplink % node_uuid  
            cmd = ("debug port --admin-state %s --port "+intf_uuid) % oper
            ret, resp = pdsctl.ExecutePdsctlCommand(node, cmd, yaml=False)
            if ret != True:
                api.Logger.error("oper:%s uplink failed at node %s : %s" %(oper, node, resp))
                return api.types.status.FAILURE      
        sleep(3) #give a short gap before printing status
        pdsctl.ExecutePdsctlShowCommand(node, "port status", yaml=False)
    return api.types.status.SUCCESS

def NaplesDataPortFlap(tc):
    naples_nodes = api.GetNaplesHostnames()
    
    api.Logger.info("shut uplinks eth1/1 and eth 1/2 on %s ..."%naples_nodes)
    ret = setDataPortState(naples_nodes, adminDown)
    
    ret = verifyDataPortState(naples_nodes, adminDown, operDown)

    api.Logger.info("no-shut uplinks eth1/1 and eth 1/2 on %s ..."%naples_nodes)
    ret = setDataPortState(naples_nodes, adminUp)
       
    ret = verifyDataPortState(naples_nodes, adminUp, operUp)
    return ret


