#! /usr/bin/python3
import iota.harness.api as api
import iota.test.apulu.utils.misc as misc_utils
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
uplinkDict = {UPLINK_PREFIX1: "Uplink0", UPLINK_PREFIX2: "Uplink1"}

def getFirstOperDownPort(node):
    misc_utils.Sleep(3)
    if GlobalOptions.dryrun:
        return api.types.status.SUCCESS

    node_uuid = EzAccessStoreClient[node].GetNodeUuid(node)
    for uplink in [UPLINK_PREFIX1, UPLINK_PREFIX2]:
        intf_uuid = uplink % node_uuid
        cmd = "port status -p "+intf_uuid
        ret, resp = pdsctl.ExecutePdsctlShowCommand(node, cmd, yaml=False)
        if ret == True and "UP          DOWN" in resp:
            return uplinkDict[uplink]

def verifyDataPortState(naples_nodes, admin, oper):
    ret = api.types.status.SUCCESS
    if GlobalOptions.dryrun:
        return ret
    retry_remaining = verifyRetry
    ret = verifyDataPortStateHelper(naples_nodes, admin, oper)
    while api.types.status.FAILURE == ret and retry_remaining > 0:
        misc_utils.Sleep(1)
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
                api.Logger.error("oper:%s uplink ret verify failed at node %s : %s" %(admin, node, resp))
                return api.types.status.FAILURE
            else :
                ret = api.types.status.SUCCESS #explicitly mark SUCCESS
            if oper == operUp:
                if "UP/UP" not in resp:
                    api.Logger.error("oper:%s uplink verify failed at node %s : %s" %(oper, node, resp))
                    return api.types.status.FAILURE
            elif admin == operDown:
                if "DOWN/DOWN" not in resp:
                    api.Logger.error("admin:%s uplink verify failed at node %s : %s" %(admin, node, resp))
                    return api.types.status.FAILURE
            else:
                if "UP/DOWN" not in resp:
                    api.Logger.error("admin:%s uplink verify failed at node %s : %s" %(admin, node, resp))
                    return api.types.status.FAILURE
    api.Logger.info("verifyDataPortState done for %s and result %d..." %(naples_nodes,ret))
    return ret
 
def switchPortFlap(tc):
    flap_count = 1
    num_ports = 2
    interval = 2
    down_time  = 2
    naples_nodes = api.GetNaplesHostnames()

    api.Logger.info("Flapping switch port on %s ..."%naples_nodes)
    ret = api.FlapDataPorts(naples_nodes, num_ports, down_time, flap_count, interval)
    if ret != api.types.status.SUCCESS:
        api.Logger.error("Failed to flap the switch port")
        return ret

    misc_utils.Sleep(60) #give a gap to for bgp to reconcile
    return api.types.status.SUCCESS

def switchPortOp(naples_nodes, oper, id):
    num_ports = 1
    start_port_id = 1
    if id == 'Switchport1':
        start_port_id = 2
    elif id == "Switchports":
        num_ports = 2
    api.Logger.info(f"Oper: {oper} for {id} on {naples_nodes} ...")
    if oper == 'down':
        ret = api.ShutDataPorts(naples_nodes, num_ports, start_port_id)
    else:
        ret = api.UnShutDataPorts(naples_nodes, num_ports, start_port_id)
    
    misc_utils.Sleep(60)  #TBD: temporary fix to wait 60 seconds for bgp sessions
    
    if ret != api.types.status.SUCCESS:
        api.Logger.error(f"Failed to bring {oper} : {id}")
        return ret

    return api.types.status.SUCCESS


def setDataPortStatePerUplink(naples_nodes, oper, id):
    uplink_list = []
    if id == 'Uplink0':
        uplink_list.append(UPLINK_PREFIX1)
    elif id == 'Uplink1':
        uplink_list.append(UPLINK_PREFIX2)
    else:
        uplink_list.append(UPLINK_PREFIX1)
        uplink_list.append(UPLINK_PREFIX2)
     
    if GlobalOptions.dryrun:
        return api.types.status.SUCCESS
       
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
        misc_utils.Sleep(1) #give a short gap before printing status
        pdsctl.ExecutePdsctlShowCommand(node, "port status", yaml=False)
    return api.types.status.SUCCESS

def setDataPortState(naples_nodes, oper):
    if GlobalOptions.dryrun:
        return api.types.status.SUCCESS
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
        misc_utils.Sleep(3) #give a short gap before printing status
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


