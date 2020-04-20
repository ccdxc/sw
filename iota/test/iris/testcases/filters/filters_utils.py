#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.address as address_utils
import iota.test.iris.utils.hal_show as hal_show_utils
import iota.test.iris.utils.host as host_utils
import iota.test.iris.utils.naples as naples_utils
import iota.test.utils.naples_host as naples_host_utils
from collections import defaultdict
import yaml

def getInterfaceVlanID(node, intf, on_naples=False):
    if on_naples:
        vlan = naples_utils.GetVlanID(node, intf)
    else:
        vlan = host_utils.GetVlanID(node, intf)
    if vlan == 0:
        vlan = 8192
    return vlan

def getParentIntf(host_intf_name):
    #TODO move to dictionary
    parent_intf_name = host_intf_name.split("_")[0]
    parent_intf_name = parent_intf_name.split(".")[0]
    return parent_intf_name

def GetNaplesHostInterfacesList(naples_node):
    #GetNaplesHostInterfaces API returns only ETH_HOST interfaces
    eth_host_intfs = list(api.GetNaplesHostInterfaces(naples_node))
    #GetHostInternalMgmtInterfaces API returns only ETH_HOST_MGMT interfaces
    eth_host_mgmt_intfs = naples_host_utils.GetHostInternalMgmtInterfaces(naples_node)
    host_intf_list = eth_host_intfs + eth_host_mgmt_intfs
    return host_intf_list

def getNaplesIntfMacAddrDict(naples_node):
    naples_intf_mac_dict = dict()

    naples_intf_list = naples_host_utils.getNaplesInterfaces(naples_node)
    for intf in naples_intf_list:
        intf_mac_addr = naples_utils.GetMACAddress(naples_node, intf)
        naples_intf_mac_dict.update({intf: intf_mac_addr})

    return naples_intf_mac_dict

def changeIntfMacAddr(node, intf_mac_dict, on_naples=False, isRollback=False):
    result = api.types.status.SUCCESS
    mac_offset = 80

    for intf, mac_addr in intf_mac_dict.items():
        if isRollback:
            mac_addr_str = mac_addr
        else:
            mac_addr_int = address_utils.convertMacStr2Dec(mac_addr)
            # New MAC = (int(Old_MAC)+ 30 + running_no)
            #TODO: Check what happens when we scale to 2k sub-if.
            mac_addr_int += mac_offset
            if api.GetNodeOs(node) == "linux":
                # In case of FreeBSD, hitting "PS-728". Based on its resolution, will remove OS check here.
                mac_offset += 1
            mac_addr_str = address_utils.formatMacAddr(mac_addr_int)
        if on_naples:
            cmd = naples_utils.SetMACAddress(node, intf, mac_addr_str)
        else:
            cmd = host_utils.SetMACAddress(node, intf, mac_addr_str)
        if cmd.exit_code != 0:
            api.Logger.critical("changeIntfMacAddr failed ", node, intf, mac_addr_str)
            api.PrintCommandResults(cmd)
            result = api.types.status.FAILURE
    return result

def getNaplesIntfEndPoints(naples_node, naples_intf_mac_dict):
    naples_ep_set = set()
    for intf, mac_addr_str in naples_intf_mac_dict.items():
        intf_mac_addr = naples_utils.GetMACAddress(naples_node, intf)
        vlan = getInterfaceVlanID(naples_node, intf, True)
        naples_ep = (vlan, intf_mac_addr, intf)
        naples_ep_set.add(naples_ep)

    return naples_ep_set

def getHostIntfEndPoints(naples_node, host_intf_mac_dict):
    host_ep_set = set()
    for intf, mac_addr_str in host_intf_mac_dict.items():
        parent_intf = getParentIntf(intf)
        intf_mac_addr = host_utils.GetMACAddress(naples_node, intf)
        vlan = getInterfaceVlanID(naples_node, intf)
        host_ep = (vlan, intf_mac_addr, parent_intf)
        host_ep_set.add(host_ep)

    return host_ep_set

def getWorkloadEndPoints(naples_node, wload_intf_mac_dict, wload_intf_vlan_map):
    wload_ep_set = set()
    #In case of classic, endpoint will be created for all mac, vlan pair for a LIF
    for intf, mac_addr_str in wload_intf_mac_dict.items():
        parent_intf = getParentIntf(intf)
        if api.GetNodeOs(naples_node) == "freebsd":
            if intf != parent_intf:
                #In case of FreeBSD, sub-if MAC won't be propagated down. so skip.
                continue
        intf_mac_addr = host_utils.GetMACAddress(naples_node, intf)
        vlan_set = wload_intf_vlan_map[parent_intf]
        for vlan in vlan_set:
            wload_ep = (vlan, intf_mac_addr, parent_intf)
            wload_ep_set.add(wload_ep)

    return wload_ep_set

def getNaplesHALEndPoints(naples_node):
    hal_ep_set = set()

    resp, result = hal_show_utils.GetHALShowOutput(naples_node, "endpoint")
    if not result:
        api.Logger.critical("unknown response from Naples")
        return hal_ep_set

    l2seg_vlan_dict = hal_show_utils.Getl2seg_vlan_mapping(naples_node)
    api.Logger.verbose("Getl2seg_vlan_mapping: l2seg_vlan ", l2seg_vlan_dict)

    ifId_lif_dict = hal_show_utils.GetIfId_lif_mapping(naples_node)
    api.Logger.verbose("GetIfId_lif_mapping: if_lif ", ifId_lif_dict)

    lifId_intfName_dict = hal_show_utils.GetLifId_intfName_mapping(naples_node)
    api.Logger.verbose("GetLifId_intfName_mapping: lif_ifName ", lifId_intfName_dict)

    cmd = resp.commands[0]
    perEPOutput = cmd.stdout.split("---")

    for ep in perEPOutput:
        epObj = yaml.load(ep, Loader=yaml.FullLoader)
        if epObj is not None:
            macAddr = epObj['spec']['keyorhandle']['keyorhandle']['endpointkey']['endpointl2l3key']['l2key']['macaddress']
            segmentid = epObj['spec']['keyorhandle']['keyorhandle']['endpointkey']['endpointl2l3key']['l2key']['l2segmentkeyhandle']['keyorhandle']['segmentid']
            interfaceid = epObj['spec']['endpointattrs']['interfacekeyhandle']['keyorhandle']['interfaceid']
            mac_addr_str = address_utils.formatMacAddr(macAddr)

            vlan_id = l2seg_vlan_dict[segmentid]
            lif_id = ifId_lif_dict[interfaceid]
            intf_name = lifId_intfName_dict[lif_id]

            hal_ep = (vlan_id, mac_addr_str, intf_name)
            hal_ep_set.add(hal_ep)

    return hal_ep_set

def getNaplesNodeandWorkloads():
    pairs = api.GetRemoteWorkloadPairs()

    workload1 = pairs[0][0]
    workload2 = pairs[0][1]
    api.Logger.verbose("workload1 : ", workload1.workload_name, workload1.node_name, workload1.uplink_vlan, id(workload1), workload1.interface, workload1.IsNaples())
    api.Logger.verbose("workload2 : ", workload2.workload_name, workload2.node_name, workload2.uplink_vlan, id(workload2), workload2.interface, workload2.IsNaples())

    naples_node = ""
    skip_tc = False

    if workload1.IsNaples():
        naples_node = workload1.node_name
    elif workload2.IsNaples():
        naples_node = workload2.node_name
    else:
        # no naples workload - so set skip to TRUE
        skip_tc = True

    return skip_tc, pairs, naples_node

def pingAllRemoteWloadPairs(workload_pairs, iterators):
    cmd_cookies = []
    req = api.Trigger_CreateExecuteCommandsRequest(serial = False)
    for pair in workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        if iterators.ipaf == 'ipv4':
            cmd_cookie = "ping -c 5 -i 0.2 -W 2000 -s %d -S %s %s" %(iterators.pktsize, w1.ip_address, w2.ip_address)
            api.Logger.verbose("Ping test cmd %s from %s(%s %s) --> %s(%s %s)" % (cmd_cookie, w1.workload_name, w1.ip_address, w1.interface, w2.workload_name, w2.ip_address, w2.interface))
        else:
            cmd_cookie = "ping6 -c 5 -i 0.2 -s %d -S %s -I %s %s" %(iterators.pktsize, w1.ipv6_address, w1.interface, w2.ipv6_address)
            api.Logger.verbose("Ping test cmd %s from %s(%s %s) --> %s(%s %s)" % (cmd_cookie, w1.workload_name, w1.ipv6_address, w1.interface, w2.workload_name, w2.ipv6_address, w2.interface))
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, cmd_cookie)
        cmd_cookies.append(cmd_cookie)

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return cmd_cookies, resp

def verifyPktFilters(intf_pktfilter_list, intf_pktfilter_dict, bc=False, mc=False, pr=False):
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
    intf_pktfilter_dict = defaultdict(list)
    resp, result = hal_show_utils.GetHALShowOutput(naples_node, "lif")
    if not result:
        api.Logger.critical("unknown response from Naples")
        return intf_pktfilter_dict, result
    cmd = resp.commands[0]

    perLifOutput = cmd.stdout.split("---")
    os = api.GetNodeOs(naples_node)
    if os == naples_host_utils.OS_TYPE_WINDOWS:
        mapping = naples_host_utils.GetWindowsPortMapping(naples_node)

    for lif in perLifOutput:
        lifObj = yaml.load(lif, Loader=yaml.FullLoader)
        if lifObj is not None:
            intfName = lifObj['spec']['name']
            #TODO: mnic interface names are appended with "/lif<lif_id>"
            # eg., inb_mnic0/lif67
            # so until that is fixed, temp hack to strip the "/lif<lif_id>" suffix
            intfName = intfName.split("/")[0]
            if os == naples_host_utils.OS_TYPE_WINDOWS and intfName[:4] =="Pen~":
                found = False
                for intf in mapping.values():
                    if intfName[4:] == intf["ifDesc"][4 - len(intfName):]:
                        intfName = intf["LinuxName"]
                        found = True
                        break
                if not found:
                    api.Logger.error("not able to find windows adapter name", intfName)
                    continue
            bc = lifObj['spec']['packetfilter']['receivebroadcast']
            mc = lifObj['spec']['packetfilter']['receiveallmulticast']
            pr = lifObj['spec']['packetfilter']['receivepromiscuous']
            pktfilter = [bc, mc, pr]
            intf_pktfilter_dict[intfName] = pktfilter

    return intf_pktfilter_dict, result

"""
   # Return mapping of native interfaces on host and all vlans under it
"""
def getHostIntf_vlan_map(node):
    host_intf_vlan_map = defaultdict(set)
    for w in api.GetWorkloads():
        if node != w.node_name: continue
        vlan = w.encap_vlan if w.encap_vlan else 8192
        host_intf_vlan_map[w.parent_interface].add(vlan)
    host_intf_list = GetNaplesHostInterfacesList(node)
    for intf in host_intf_list:
        vlan = getInterfaceVlanID(node, intf)
        host_intf_vlan_map[intf].add(vlan)
    return host_intf_vlan_map

"""
   # Return mapping of native & tagged interfaces on host and
   #        their corresponding parent interface
"""
def getHostIntf_parentIntf_map(node):
    host_intf_parentIntf_map = defaultdict()
    for w in api.GetWorkloads():
        if node != w.node_name: continue
        host_intf_parentIntf_map[w.interface] = w.parent_interface
    host_intf_list = GetNaplesHostInterfacesList(node)
    for intf in host_intf_list:
        host_intf_parentIntf_map[intf] = intf
    return host_intf_parentIntf_map

"""
   # Return set of workload interfaces on naples_node
"""
def getNaplesWorkload_Intf(naples_node):
    wload_intf_set = set()
    for w in api.GetWorkloads():
        if naples_node != w.node_name: continue
        wload_intf_set.add(w.interface)
    return wload_intf_set

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
        epObj = yaml.load(ep, Loader=yaml.FullLoader)
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

def add_mc_mac_for_all_vlans(mac_addr_str):
    #TODO: Make it proper API
    if address_utils.is_gvrp_mcast_mac(mac_addr_str):
        return True
    if "33:33:ff:00:00:01" == mac_addr_str:
        return False
    if address_utils.is_all_node_mc_mac(mac_addr_str):
        return False
    if "01:00:5e:00:00:01" == mac_addr_str:
        return False

    return True

def getHostIntfMcastEndPoints(naples_node):
    host_mc_ep_set = set()

    #Get interface and its parent interface mapping
    host_intf_parentIntf_map = getHostIntf_parentIntf_map(naples_node)
    #Get a map of native interface and all vlans under it
    host_intf_vlan_map = getHostIntf_vlan_map(naples_node)
    #Get a set of all workload interfaces on our Naples node
    wload_intf_set = getNaplesWorkload_Intf(naples_node)
    #Get a set of all native interfaces in host (which has naples node)
    host_intf_set = set(GetNaplesHostInterfacesList(naples_node))
    #Final list of interfaces is union of workload interfaces and host interfaces
    host_intf_list = list(wload_intf_set.union(host_intf_set))

    api.Logger.verbose("getHostIntfMcastEndPoints: host_intf_parentIntf_map ", host_intf_parentIntf_map)
    api.Logger.verbose("getHostIntfMcastEndPoints: host_intf_vlan_map ", host_intf_vlan_map)
    api.Logger.verbose("getHostIntfMcastEndPoints: host_intf_list ", host_intf_list)

    for intf in host_intf_list:
        # Get vlan of this interface
        vlan_id = getInterfaceVlanID(naples_node, intf)
        # Get parent interface for this interface
        parent_intf = host_intf_parentIntf_map[intf]
        vlan_set = host_intf_vlan_map[parent_intf]
        mcastMAC_list = host_utils.GetMcastMACAddress(naples_node, intf)
        for mac in mcastMAC_list:
            if add_mc_mac_for_all_vlans(mac):
                for vlan in vlan_set:
                    # Add an endpoint for all vlans in this LIF
                    host_mc_ep = (vlan, mac, parent_intf)
                    host_mc_ep_set.add(host_mc_ep)
                pass
            else:
                host_mc_ep = (vlan_id, mac, parent_intf)
                host_mc_ep_set.add(host_mc_ep)
    return host_mc_ep_set

def getNaplesIntfMcastEndPoints(naples_node):
    naples_mc_ep_set = set()
    # Get a list of interfaces on Naples ARM
    naples_intf_list = naples_host_utils.getNaplesInterfaces(naples_node)
    for intf in naples_intf_list:
        vlan_id = getInterfaceVlanID(naples_node, intf, True)
        mcastMAC_list = naples_utils.GetMcastMACAddress(naples_node, intf)
        for mac in mcastMAC_list:
            naples_mc_ep = (vlan_id, mac, intf)
            naples_mc_ep_set.add(naples_mc_ep)
    return naples_mc_ep_set

def verifyEndpoints(host_view, hal_view):
    #Get the symmetric difference between the two views
    diff = host_view ^ hal_view

    if len(diff) == 0:
        result = True
    else:
        # If there is a difference in view, then return failure
        result = False
        api.Logger.error("Failure - verifyEndpoints failed ", len(diff), diff)
        # endpoints missing in HAL
        diff_1 = host_view - hal_view
        api.Logger.error("Endpoints present in host but NOT in hal ", len(diff_1), diff_1)
        # stale endpoints in HAL
        diff_2 = hal_view - host_view
        if len(diff_2) == 1:
            for (vlan, mac, intf) in diff_2:
                if intf.find('swm_lif') != -1:
                    api.Logger.info("SWM endpoints present in hal but NOT in host as expected", len(diff_2), diff_2)
                    result = True
        else:
            api.Logger.error("Endpoints present in hal but NOT in host ", len(diff_2), diff_2)

    return result
