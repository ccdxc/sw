#! /usr/bin/python3
import iota.harness.api as api
import iota.test.iris.utils.address as address_utils
import iota.test.iris.utils.hal_show as hal_show_utils
import iota.test.iris.utils.host as host_utils
import iota.test.iris.utils.naples as naples_utils
import iota.test.iris.utils.naples_host as naples_host_utils
import yaml
import re

def getInterfaceVlanID(node, intf, on_naples=False):
    if on_naples:
        vlan = naples_utils.GetVlanID(node, intf)
    else:
        vlan = host_utils.GetVlanID(node, intf)
    if vlan is 0:
        vlan = 8192
    return vlan

def getParentIntf(host_intf_name):
    intf_map_pattern = [['enp182s0\S*', 'enp182s0'], ['enp183s0\S*', 'enp183s0'], ['ionic1\S*', 'ionic1'], ['ionic2\S*', 'ionic2'], ['enp181s0\S*', 'enp181s0'], ['ionic0\S*', 'ionic0']]
    for pattern in intf_map_pattern:
        match = re.match(pattern[0], host_intf_name)
        if match:
            return pattern[1]
    api.Logger.error("UC MAC : Failure - No parent intf found for ", host_intf_name)
    return "NULL"

def getNaplesView_of_Host_Intf(host_intf_name):
    #TODO: TEMP hack until 'PS-603' is resolved
    intf_map_pattern = [['enp182s0\S*', 'eth1'], ['enp183s0', 'eth_mgmt0'], ['ionic1\S*', 'eth1'], ['ionic2', 'eth_mgmt0'], ['enp181s0\S*', 'eth0'], ['ionic0\S*', 'eth0']]
    for pattern in intf_map_pattern:
        match = re.match(pattern[0], host_intf_name)
        if match:
            return pattern[1]
    api.Logger.error("UC MAC : Failure - No pattern found for interface ", host_intf_name)
    return "NULL"

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
            api.Logger.error("changeIntfMacAddr failed ", node, intf, mac_addr_str)
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
        #TODO: To be removed once "#PS-603" is fixed
        intf_name = getNaplesView_of_Host_Intf(intf)
        intf_mac_addr = host_utils.GetMACAddress(naples_node, intf)
        vlan = getInterfaceVlanID(naples_node, intf)
        host_ep = (vlan, intf_mac_addr, intf_name)
        host_ep_set.add(host_ep)
    
    return host_ep_set

def getWorkloadEndPoints(naples_node, wload_intf_mac_dict, wload_intf_vlan_map):
    wload_ep_set = set()
    #In case of classic, endpoint will be created for all mac, vlan pair for a LIF
    for intf, mac_addr_str in wload_intf_mac_dict.items():
        #TODO: To be removed once "#PS-603" is fixed
        intf_name = getNaplesView_of_Host_Intf(intf)
        intf_mac_addr = host_utils.GetMACAddress(naples_node, intf)
        parent_intf = getParentIntf(intf)
        vlan_set = wload_intf_vlan_map[parent_intf]
        for vlan in vlan_set:
            wload_ep = (vlan, intf_mac_addr, intf_name)
            wload_ep_set.add(wload_ep)

    return wload_ep_set

def getNaplesHALEndPoints(naples_node):
    hal_ep_set = set()

    resp, result = hal_show_utils.GetHALShowOutput(naples_node, "endpoint")
    if not result:
        api.Logger.error("unknown response from Naples")
        return endpoint_dict

    l2seg_vlan_dict = hal_show_utils.Getl2seg_vlan_mapping(naples_node)
    #api.Logger.info("UC MAC filter : TEST l2seg_vlan ", l2seg_vlan_dict)

    ifId_lif_dict = hal_show_utils.GetIfId_lif_mapping(naples_node)
    #api.Logger.info("UC MAC filter : TEST if_lif ", ifId_lif_dict)

    lifId_intfName_dict = hal_show_utils.GetLifId_intfName_mapping(naples_node)
    #api.Logger.info("UC MAC filter : TEST lif_ifName ", lifId_intfName_dict)

    cmd = resp.commands[0]
    perEPOutput = cmd.stdout.split("---")

    for ep in perEPOutput:
        epObj = yaml.load(ep)
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
    api.Logger.info("workload1 : ", workload1.workload_name, workload1.node_name, workload1.uplink_vlan, id(workload1), workload1.interface, workload1.IsNaples())
    api.Logger.info("workload2 : ", workload2.workload_name, workload2.node_name, workload2.uplink_vlan, id(workload2), workload2.interface, workload2.IsNaples())

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
    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)
    for pair in workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        if iterators.ipaf == 'ipv4':
            cmd_cookie = "ping -c 10 -i 0.2 -s %d -S %s %s" %(iterators.pktsize, w1.ip_address, w2.ip_address)
            api.Logger.info("Ping test cmd %s from %s(%s %s) --> %s(%s %s)" % (cmd_cookie, w1.workload_name, w1.ip_address, w1.interface, w2.workload_name, w2.ip_address, w2.interface))
        else:
            cmd_cookie = "ping6 -c 10 -i 0.2 -s %d -S %s -I %s %s" %(iterators.pktsize, w1.ipv6_address, w1.interface, w2.ipv6_address)
            api.Logger.info("Ping test cmd %s from %s(%s %s) --> %s(%s %s)" % (cmd_cookie, w1.workload_name, w1.ipv6_address, w1.interface, w2.workload_name, w2.ipv6_address, w2.interface))
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name, cmd_cookie)
        cmd_cookies.append(cmd_cookie)

    trig_resp = api.Trigger(req)
    term_resp = api.Trigger_TerminateAllCommands(trig_resp)
    resp = api.Trigger_AggregateCommandsResponse(trig_resp, term_resp)
    return cmd_cookies, resp
