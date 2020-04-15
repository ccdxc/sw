#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.debug as debug_utils
import iota.test.iris.utils.host as host_utils
import iota.test.iris.utils.traffic as traffic_utils
import iota.test.iris.testcases.filters.filters_utils as filters_utils
from collections import defaultdict
import time

def getAllEndPointsView(tc):
    # workload endpoints
    wload_ep_set = filters_utils.getWorkloadEndPoints(tc.naples_node, tc.wload_intf_mac_dict, tc.wload_intf_vlan_map)
    
    # Other host interface endpoints (which aren't part of workloads)
    host_ep_set = filters_utils.getHostIntfEndPoints(tc.naples_node, tc.host_intf_mac_dict)

    # Naples intf endpoints
    naples_ep_set = filters_utils.getNaplesIntfEndPoints(tc.naples_node, tc.naples_intf_mac_dict)

    # HAL view of endpoints
    hal_ep_set = filters_utils.getNaplesHALEndPoints(tc.naples_node)

    #Keeping them separate as it is useful for debugging in scale
    api.Logger.debug("getAllEndPointsView: wload_ep_set ", len(wload_ep_set), wload_ep_set)
    api.Logger.debug("getAllEndPointsView: host_ep_set ", len(host_ep_set), host_ep_set)
    api.Logger.debug("getAllEndPointsView: naples_ep_set ", len(naples_ep_set), naples_ep_set)
    api.Logger.debug("getAllEndPointsView: hal_ep_set ", len(hal_ep_set), hal_ep_set)

    return wload_ep_set, host_ep_set, naples_ep_set, hal_ep_set

def getNaplesWorkloadInfo(naples_node):
    wload_intf_list = list()
    wload_intf_vlan_map = defaultdict(set)
    for w in api.GetWorkloads():
        api.Logger.verbose("UC MAC workload : ", w.workload_name, w.node_name, w.uplink_vlan, id(w), w.encap_vlan, w.interface, w.ip_prefix, w.mac_address, w.IsNaples())
        if not w.IsNaples(): continue
        if naples_node != w.node_name: continue
        wload_intf_list.append(w.interface)
        vlan = w.encap_vlan if w.encap_vlan else 8192
        wload_intf_vlan_map[w.parent_interface].add(vlan)

    wload_intf_list.sort()

    return wload_intf_list, wload_intf_vlan_map

def verifyEndPoints(tc):
    wload_ep_view = tc.wload_ep_set
    host_ep_view = tc.host_ep_set
    naples_ep_view = tc.naples_ep_set
    hal_ep_view = tc.hal_ep_set

    # HAL's view of endpoints = Union of workload + Host + Naples Intf 
    host_view = wload_ep_view | host_ep_view | naples_ep_view
    return filters_utils.verifyEndpoints(host_view, hal_ep_view)

def changeMacAddrTrigger(tc, isRollback=False):
    result = api.types.status.SUCCESS
    node = tc.naples_node

    # change host MAC as last one to avoid ssh failure
    # reset host MAC first
    if isRollback:
        result1 = filters_utils.changeIntfMacAddr(node, tc.host_intf_mac_dict, False, isRollback)
        result2 = filters_utils.changeIntfMacAddr(node, tc.wload_intf_mac_dict, False, isRollback)
        result3 = filters_utils.changeIntfMacAddr(node, tc.naples_intf_mac_dict, True, isRollback)
    else:
        #Change MAC of workload interfaces
        result1 = filters_utils.changeIntfMacAddr(node, tc.wload_intf_mac_dict, False, isRollback)
        #Change MAC of naples interfaces
        result2 = filters_utils.changeIntfMacAddr(node, tc.naples_intf_mac_dict, True, isRollback)
        #Change MAC of other host interfaces
        result3 = filters_utils.changeIntfMacAddr(node, tc.host_intf_mac_dict, False, isRollback)

    if any([result1, result2, result3]) is True:
        api.Logger.error("UC MAC filter : Trigger -> changeMacAddrTrigger failed ", result1, result2, result3, isRollback)
        result = api.types.status.FAILURE

    return result

def Setup(tc):
    api.Logger.verbose("UC MAC filter : Setup")
    tc.skip = False
    result = api.types.status.SUCCESS

    tc.skip, tc.workloads, naples_node = filters_utils.getNaplesNodeandWorkloads()
    if tc.skip:
        api.Logger.error("UC MAC filter : Setup -> No Naples Topology - So skipping the TC")
        return api.types.status.IGNORED

    #Get MAC address of all the naples' interfaces for rollback later
    naples_intf_mac_dict = filters_utils.getNaplesIntfMacAddrDict(naples_node)

    #Get workload interfaces for rollback later
    wloadIntf_list, wload_intf_vlan_map = getNaplesWorkloadInfo(naples_node)
            
    #Keep a dict of (wloadIntf, mac) pair for rollback later
    wload_intf_mac_dict = dict()

    #Get MAC address of all the workload interfaces
    for intf in wloadIntf_list:
        intf_mac_addr = host_utils.GetMACAddress(naples_node, intf)
        wload_intf_mac_dict.update({intf: intf_mac_addr})

    host_intf_mac_dict = dict()
    #Get MAC address of all the interfaces on the host of Naples (except the workload interfaces)
    host_intf_list = filters_utils.GetNaplesHostInterfacesList(naples_node)
    api.Logger.verbose("UC MAC filter : Setup host_intf_list : ", host_intf_list)
    for intf in host_intf_list:
        if intf not in wload_intf_mac_dict:
            intf_mac_addr = host_utils.GetMACAddress(naples_node, intf)
            host_intf_mac_dict.update({intf: intf_mac_addr})

    #Store the following info for building endpoint view and also for rollback
    tc.naples_node = naples_node
    tc.naples_intf_mac_dict = naples_intf_mac_dict
    tc.host_intf_mac_dict = host_intf_mac_dict
    tc.wload_intf_mac_dict = wload_intf_mac_dict
    tc.wload_intf_vlan_map = wload_intf_vlan_map

    api.Logger.debug("UC MAC filter : Setup naples_intf_mac_dict : ", naples_intf_mac_dict)
    api.Logger.debug("UC MAC filter : Setup host_intf_mac_dict : ", host_intf_mac_dict)
    api.Logger.debug("UC MAC filter : Setup wload_intf_mac_dict : ", wload_intf_mac_dict)
    api.Logger.debug("UC MAC filter : Setup wload_intf_vlan_map : ", wload_intf_vlan_map)

    api.Logger.info("UC MAC filter : Setup final result - ", result)
    debug_utils.collect_showtech(result)
    return result

def Trigger(tc):
    api.Logger.verbose("UC MAC filter : Trigger")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    if tc.iterators.mac_change:
        result = changeMacAddrTrigger(tc)
        time.sleep(5)
        api.Logger.debug("UC MAC filter : Trigger -> Change MAC addresses result ", result)
    else:
        api.Logger.debug("UC MAC filter : Trigger -> NO Change to MAC addresses")

    # Triggers done - Now build endpoint view of Host and Naples
    tc.wload_ep_set, tc.host_ep_set, tc.naples_ep_set, tc.hal_ep_set = getAllEndPointsView(tc)

    tc.cmd_cookies, tc.resp = traffic_utils.pingAllRemoteWloadPairs(mtu=tc.iterators.pktsize, af=str(tc.iterators.ipaf))

    api.Logger.info("UC MAC filter : Trigger final result - ", result)
    debug_utils.collect_showtech(result)
    return result

def Verify(tc):
    '''
    #TODO
        1. any HW table dump to check? - Not needed apparently
            Ideally, We can also check registered MAC table as follows
            halctl show table dump --entry-id 1840 --table-id 4
            Get the entry id from "halctl show endpoint --yaml"
        2. Check for memleaks [will pull from Amrita's TC]
            2.1 "halctl show system memory slab | grep ep"
    '''
    api.Logger.verbose("UC MAC filter : Verify")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED
    cookie_idx = 0

    # Check if MACs in "halctl show endpoint" match with host & workload interface MAC
    if not verifyEndPoints(tc):
        api.Logger.error("UC MAC filter : Verify failed for verifyEndPoints")
        result = api.types.status.FAILURE
        debug_utils.collect_showtech(result)
        return result
    else:
        api.Logger.debug("UC MAC filter : Verify - verifyEndPoints SUCCESS ")
    
    """
       # If "filters" bundle run as first bundle in a suite,
       # there are chances that STP might not have converged yet and as a result of that
       # first couple of remote workload pings might fail.
       # Solutions are
           1. Modify n3k configs for reduced STP convergence
           2. Increase wait time between pings
           3. Add sleep before starting the trigger
       # Last 2 solutions mean increase in script execution time which is a big NO.
       # so leaving it for now as this will not happen in sanity
    """
    result = traffic_utils.verifyPing(tc.cmd_cookies, tc.resp)
    
    api.Logger.info("UC MAC filter : Verify final result - ", result)
    debug_utils.collect_showtech(result)
    return result

def Teardown(tc):
    api.Logger.verbose("UC MAC filter : Teardown")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    if tc.iterators.mac_change:
        result = changeMacAddrTrigger(tc, True)
        api.Logger.debug("UC MAC filter : Teardown -> rolling back MAC address changes ", result)
    else:
        api.Logger.debug("UC MAC filter : Teardown -> NO rollback")

    api.Logger.info("UC MAC filter : Teardown final result - ", result)
    debug_utils.collect_showtech(result)
    return result
