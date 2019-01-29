#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.naples_host as naples_host_utils
import iota.test.iris.utils.hal_show as hal_show_utils
import iota.test.iris.testcases.filters.filters_utils as filters_utils
from collections import defaultdict
import yaml

def verifyPktFilters(intf_pktfilter_list, intf_pktfilter_dict, bc=False, mc=False, pr=False):
    #TODO: Move to filters_utils.py
    result = True
    for intf in intf_pktfilter_list:
        if intf not in intf_pktfilter_dict:
            # Interface not found in "halctl show lif"
            result = False 
            api.Logger.error("Failure - verifyPktFilters failed ", intf)
            continue
        pktfilter = intf_pktfilter_dict[intf]
        #pktfilter is list of packet filters in this order: [Broadcast, Multicast, Promiscuous]
        if bc and not pktfilter[0]:
            result = False 
            api.Logger.error("Failure - verifyBCPktFilters failed ", intf, pktfilter)
        if mc and not pktfilter[1]:
            result = False 
            api.Logger.error("Failure - verifyMCPktFilters failed ", intf, pktfilter)
        if pr and not pktfilter[2]:
            result = False 
            api.Logger.error("Failure - verifyPRPktFilters failed ", intf, pktfilter)

    return result

def getAllIntfPktFilter(naples_node):
    #TODO: Move to filters_utils.py
    intf_pktfilter_dict = defaultdict(list)
    resp, result = hal_show_utils.GetHALShowOutput(naples_node, "lif")
    if not result:
        api.Logger.error("unknown response from Naples")
        return intf_pktfilter_dict, result
    cmd = resp.commands[0]

    perLifOutput = cmd.stdout.split("---")

    for lif in perLifOutput:
        lifObj = yaml.load(lif)
        if lifObj is not None:
            intfName = lifObj['spec']['name']
            bc = lifObj['spec']['packetfilter']['receivebroadcast']
            mc = lifObj['spec']['packetfilter']['receiveallmulticast']
            pr = lifObj['spec']['packetfilter']['receivepromiscuous']
            pktfilter = [bc, mc, pr]
            intf_pktfilter_dict[intfName] = pktfilter

    return intf_pktfilter_dict, result

def getInterfaceList(naples_node):
    intf_pktfilter_list= list()
    naples_intf_list = naples_host_utils.getNaplesInterfaces(naples_node)
    host_intf_list = list(api.GetNaplesHostInterfaces(naples_node))

    for intf in host_intf_list:
        intf_name = filters_utils.getNaplesView_of_Host_Intf(intf)
        intf_pktfilter_list.append(intf_name)

    intf_pktfilter_list.extend(naples_intf_list)

    api.Logger.info("BC MAC filter : Setup host_intf_list : ", host_intf_list)
    api.Logger.info("BC MAC filter : Setup naples_intf_list : ", naples_intf_list)
    api.Logger.info("BC MAC filter : Setup intf_pktfilter_dict : ", intf_pktfilter_list)

    return intf_pktfilter_list

def Setup(tc):
    api.Logger.info("BC MAC filter : Setup")
    tc.skip = False
    result = api.types.status.SUCCESS

    tc.skip, tc.workloads, naples_node = filters_utils.getNaplesNodeandWorkloads()
    if tc.skip:
        api.Logger.error("BC MAC filter : Setup -> No Naples Topology - So skipping the TC")
        return api.types.status.IGNORED
    
    intf_pktfilter_list = getInterfaceList(naples_node)

    tc.naples_node = naples_node
    tc.intf_pktfilter_list = intf_pktfilter_list

    api.Logger.info("BC MAC filter : Setup final_intf_list : ", tc.intf_pktfilter_list)
    api.Logger.info("BC MAC filter : Setup final result - ", result)
    return result

def Trigger(tc):
    api.Logger.info("BC MAC filter : Trigger")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    # check BC flag
    tc.intf_pktfilter_dict, res = getAllIntfPktFilter(tc.naples_node)
    if not res:
        result = api.types.status.FAILURE
        api.Logger.error("BC MAC filter : Trigger failed for getAllIntfPktFilter ", res)
        return result

    #tc.cmd_cookies, tc.resp = filters_utils.pingAllRemoteWloadPairs(tc.workloads, tc.iterators)

    api.Logger.info("BC MAC filter : Trigger final result - ", result)
    return result

def Verify(tc):
    '''
    #TODO
        1. any HW table dump to check? - Not needed apparently
        2. Check for memleaks [will pull from Amrita's TC]
    '''
    api.Logger.info("BC MAC filter : Verify")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED
    cookie_idx = 0

    # Check if all interfaces have broadcast filter enabled on them
    if not verifyPktFilters(tc.intf_pktfilter_list, tc.intf_pktfilter_dict, True):
        api.Logger.error("BC MAC filter : Verify failed for verifyBCPktFilters ", tc.intf_pktfilter_dict)
        result = api.types.status.FAILURE
        return result
    else:
        api.Logger.info("BC MAC filter : Verify - verifyBCPktFilters SUCCESS ")
    
    return result
    #TODO uncomment above on adding ping

    if tc.resp is None:
        api.Logger.error("BC MAC filter : Verify failed - no response")
        result = api.types.status.FAILURE
        return result

    for cmd in tc.resp.commands:
        api.Logger.info("Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0 and not api.Trigger_IsBackgroundCommand(cmd):
            api.Logger.error("BC MAC filter : Verify failed for %s" % (tc.cmd_cookies[cookie_idx]))
            api.PrintCommandResults(cmd)
            result = api.types.status.FAILURE
        cookie_idx += 1
    
    api.Logger.info("BC MAC filter : Verify final result - ", result)

    return result

def Teardown(tc):
    api.Logger.info("BC MAC filter : Teardown")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    api.Logger.info("BC MAC filter : Teardown final result - ", result)

    return result
