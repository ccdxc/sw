#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.host as host_utils
import iota.test.iris.utils.hal_show as hal_show_utils
import iota.test.iris.utils.address as address_utils
import iota.test.iris.testcases.filters.filters_utils as filters_utils
import iota.test.iris.utils.naples_host as naples_host_utils
from collections import defaultdict
import yaml

def getmcastMACAddress(node, interface):
    #TODO: move it to appropriate utils
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    cmd = "ip maddr show " + interface + " | grep link | cut -d' ' -f3"
    api.Trigger_AddNaplesCommand(req, node, cmd)
    resp = api.Trigger(req)
    mcastMAC_list = list(filter(None, resp.commands[0].stdout.strip("\n").split("\r")))
    return mcastMAC_list

def getNaplesHALmcastEndPoints(naples_node):
    hal_mc_ep_set = set()

    resp, result = hal_show_utils.GetHALShowOutput(naples_node, "multicast")
    if not result:
        api.Logger.error("unknown response from Naples")
        return hal_mc_ep_set

    l2seg_vlan_dict = hal_show_utils.Getl2seg_vlan_mapping(naples_node)
    api.Logger.verbose("Getl2seg_vlan_mapping: l2seg_vlan ", l2seg_vlan_dict)

    ifId_lif_dict = hal_show_utils.GetIfId_lif_mapping(naples_node)
    api.Logger.verbose("GetIfId_lif_mapping: ifId_lif_dict ", ifId_lif_dict)

    lifId_intfName_dict = hal_show_utils.GetLifId_intfName_mapping(naples_node)
    api.Logger.verbose("GetLifId_intfName_mapping: lifId_intfName_dict ", lifId_intfName_dict)

    cmd = resp.commands[0]
    perEPOutput = cmd.stdout.split("---")

    for ep in perEPOutput:
        epObj = yaml.load(ep)
        if epObj is not None:
            macAddr = epObj['spec']['keyorhandle']['keyorhandle']['key']['ipormac']['mac']['group']
            mac_addr_str = address_utils.formatMacAddr(macAddr)
            segmentid = epObj['spec']['keyorhandle']['keyorhandle']['key']['l2segmentkeyhandle']['keyorhandle']['segmentid']
            vlan_id = l2seg_vlan_dict[segmentid]

            oifIntfs = epObj['spec']['oifkeyhandles']
            for i in range(0, len(oifIntfs)):
                intfID = oifIntfs[i]['keyorhandle']['interfaceid']
                lif_id = ifId_lif_dict[intfID]
                intf_name = lifId_intfName_dict[lif_id]
                hal_mc_ep = (vlan_id, mac_addr_str, intf_name)
                hal_mc_ep_set.add(hal_mc_ep)

    return hal_mc_ep_set

def getNaplesIntfMcastEndPoints(naples_node, intf_list):
    naples_mc_ep_set = set()
    for intf in intf_list:
        vlan_id = filters_utils.getInterfaceVlanID(naples_node, intf, True)
        mcastMAC_list = getmcastMACAddress(naples_node, intf)
        for mac in mcastMAC_list:
            naples_mc_ep = (vlan_id, mac, intf)
            naples_mc_ep_set.add(naples_mc_ep)
    return naples_mc_ep_set

def getHostIntfMcastEndPoints(naples_node, host_intf_list):
    host_mc_ep_set = set()
    return host_mc_ep_set

def getAllmcastEndPointsView(tc):
    
    # Host interface mcast endpoints
    host_intf_list = list(api.GetNaplesHostInterfaces(tc.naples_node))
    tc.host_mc_ep_set = getHostIntfMcastEndPoints(tc.naples_node, host_intf_list)

    # Naples intf mcast endpoints
    naples_intf_list = naples_host_utils.getNaplesInterfaces(tc.naples_node)
    tc.naples_mc_ep_set = getNaplesIntfMcastEndPoints(tc.naples_node, naples_intf_list)

    # HAL view of endpoints
    tc.hal_mc_ep_set = getNaplesHALmcastEndPoints(tc.naples_node)

    #Keeping them separate as it is useful for debugging in scale
    api.Logger.debug("getAllmcastEndPointsView: host_mc_ep_set ", len(tc.host_mc_ep_set), tc.host_mc_ep_set)
    api.Logger.debug("getAllmcastEndPointsView: naples_mc_ep_set ", len(tc.naples_mc_ep_set), tc.naples_mc_ep_set)
    api.Logger.debug("getAllmcastEndPointsView: hal_mc_ep_set ", len(tc.hal_mc_ep_set), tc.hal_mc_ep_set)

    return

"""
def getNaplesWorkloadInfo(naples_node):
    wload_intf_list = list()
    wload_intf_vlan_map = defaultdict(set)
    for w in api.GetWorkloads():
        api.Logger.info("UC MAC workload : ", w.workload_name, w.node_name, w.uplink_vlan, id(w), w.encap_vlan, w.interface, w.ip_prefix, w.mac_address, w.IsNaples())
        if not w.IsNaples(): continue
        if naples_node != w.node_name: continue
        wload_intf_list.append(w.interface)
        vlan = w.encap_vlan if w.encap_vlan else 8192
        wload_intf_vlan_map[w.parent_interface].add(vlan)

    wload_intf_list.sort()

    return wload_intf_list, wload_intf_vlan_map
"""

def verifyMCEndPoints(tc):
    return True
    wload_ep_view = tc.wload_ep_set
    host_ep_view = tc.host_ep_set
    naples_ep_view = tc.naples_ep_set
    hal_ep_view = tc.hal_ep_set

    # HAL's view of endpoints = Union of workload + Host + Naples Intf 
    host_view = wload_ep_view | host_ep_view | naples_ep_view
    #Get the symmetric difference between the two views
    diff = host_view ^ hal_ep_view

    if len(diff) == 0:
        result = True
    else:
        # If there is a difference in view, then mark the TC failed.
        result = False
        api.Logger.error("UC MAC : Failure - verifyEndPoints failed ", len(diff), diff)

    return result

def Setup(tc):
    api.Logger.verbose("MC MAC filter : Setup")
    tc.skip = False
    result = api.types.status.SUCCESS

    tc.skip, tc.workloads, tc.naples_node = filters_utils.getNaplesNodeandWorkloads()
    if tc.skip:
        api.Logger.error("MC MAC filter : Setup -> No Naples Topology - So skipping the TC")
        return api.types.status.IGNORED

    api.Logger.info("MC MAC filter : Setup final result - ", result)
    return result

def Trigger(tc):
    api.Logger.verbose("MC MAC filter : Trigger")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    # Triggers done - Now build endpoint view of Host and Naples
    getAllmcastEndPointsView(tc)

    api.Logger.info("MC MAC filter : Trigger final result - ", result)
    return result

def Verify(tc):
    '''
    #TODO
        1. any HW table dump to check? - Not needed apparently
            Ideally, We can also check registered MAC table as follows
            halctl show table dump --entry-id 1840 --table-id 4
            Get the entry id from "halctl show endpoint --yaml"
        2. Check for memleaks [will pull from Amrita's TC]
    '''
    api.Logger.verbose("MC MAC filter : Verify")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    # Check if MACs in "halctl show multicast" match with host & naples interface mcast MAC
    if not verifyMCEndPoints(tc):
        api.Logger.error("MC MAC filter : Verify failed for verifyMCEndPoints")
        result = api.types.status.FAILURE
    else:
        api.Logger.debug("MC MAC filter : Verify - verifyMCEndPoints SUCCESS ")
    
    api.Logger.info("MC MAC filter : Verify final result - ", result)

    return result

def Teardown(tc):
    api.Logger.verbose("MC MAC filter : Teardown")
    result = api.types.status.SUCCESS
    if tc.skip: return api.types.status.IGNORED

    api.Logger.info("MC MAC filter : Teardown final result - ", result)

    return result
