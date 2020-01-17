#! /usr/bin/python3
import pdb
import time
import iota.protos.pygen.topo_svc_pb2 as topo_svc
import iota.harness.api as api
import iota.test.utils.naples_host as utils
import iota.test.iris.utils.subif_utils as subif_utils
import iota.test.iris.testcases.filters.filters_utils as filters_utils
import iota.test.iris.utils.host as util_host

def InitializeMacConfig(tc):
    nodes = api.GetNaplesHostnames()
    naples_node = nodes[0]

    # workload endpoints
    wload_intf_mac_dict = {}
    wload_intf_vlan_map = {}
    for wd in subif_utils.getNativeWorkloads():
        if wd.node_name == naples_node and wd.interface == wd.parent_interface:
            wload_intf_mac_dict[wd.interface] = util_host.GetMACAddress(naples_node, wd.interface)
            wload_intf_vlan_map[wd.interface] = [8192]
            for sub in subif_utils.GetSubifs(wd.interface, wd.node_name):
                sub_wd = subif_utils.getWorkloadForInf(sub, wd.node_name)
                wload_intf_mac_dict[sub_wd.interface] = util_host.GetMACAddress(naples_node, sub_wd.interface)
                lst = wload_intf_vlan_map.get(wd.interface, None)
                if lst:
                    (wload_intf_vlan_map[wd.interface]).append(sub_wd.encap_vlan)
                else:
                    (wload_intf_vlan_map[wd.interface]).append(sub_wd.encap_vlan)

    host_intf_mac_dict = {}
    for wl in api.GetWorkloads():
        if wl.node_name == naples_node and wl.interface == wl.parent_interface:
            if wl.interface not in wload_intf_mac_dict:
                host_intf_mac_dict[wl.interface] = util_host.GetMACAddress(naples_node, wl.interface)

    for inf in utils.GetHostInternalMgmtInterfaces(naples_node):
        if inf not in wload_intf_mac_dict:
            mac = util_host.GetMACAddress(naples_node, inf)
            host_intf_mac_dict[inf] = mac

    naples_intf_mac_dict = filters_utils.getNaplesIntfMacAddrDict(naples_node)

    #Store the following info for building endpoint view and also for rollback
    tc.naples_node = naples_node
    tc.naples_intf_mac_dict = naples_intf_mac_dict
    tc.host_intf_mac_dict = host_intf_mac_dict
    tc.wload_intf_mac_dict = wload_intf_mac_dict
    tc.wload_intf_vlan_map = wload_intf_vlan_map


def __create_subifs(subif_count = 0, native_inf = None):
    for wl in api.GetWorkloads():
        if wl.parent_interface != wl.interface:
            continue
        if wl.IsNaples():
            subif_utils.Create_Subifs(subif_count, wl.interface, wl.node_name)

def __delete_subifs(h_interface = None, node_name = None):
    for wl in api.GetWorkloads():
        if wl.parent_interface != wl.interface:
            continue
        if wl.IsNaples():
            subif_utils.Delete_Subifs(wl.interface, wl.node_name)

def ValidateMacRegistration(tc):
    wload_ep_set = filters_utils.getWorkloadEndPoints(tc.naples_node, tc.wload_intf_mac_dict, tc.wload_intf_vlan_map)

    # Other host interface endpoints (which aren't part of workloads)
    host_ep_set = filters_utils.getHostIntfEndPoints(tc.naples_node, tc.host_intf_mac_dict)

    # Naples intf endpoints
    naples_ep_set = filters_utils.getNaplesIntfEndPoints(tc.naples_node, tc.naples_intf_mac_dict)

    # HAL view of endpoints
    hal_ep_set = filters_utils.getNaplesHALEndPoints(tc.naples_node)

    #Keeping them separate as it is useful for debugging in scale
    api.Logger.info("getAllEndPointsView: wload_ep_set ", len(wload_ep_set), wload_ep_set)
    api.Logger.info("getAllEndPointsView: host_ep_set ", len(host_ep_set), host_ep_set)
    api.Logger.info("getAllEndPointsView: naples_ep_set ", len(naples_ep_set), naples_ep_set)
    api.Logger.info("getAllEndPointsView: hal_ep_set ", len(hal_ep_set), hal_ep_set)

    return wload_ep_set, host_ep_set, naples_ep_set, hal_ep_set

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
    #Change MAC of workload interfaces
    result1 = filters_utils.changeIntfMacAddr(node, tc.wload_intf_mac_dict, False, isRollback)
    #Change MAC of other host interfaces
    result2 = filters_utils.changeIntfMacAddr(node, tc.host_intf_mac_dict, False, isRollback)
    #Change MAC of naples interfaces
    #TODO changing MAC address of mnic results in IOTA failure in FreeBSD. commenting this out until its RCA
    if api.GetNodeOs(node) == "linux":
        result3 = filters_utils.changeIntfMacAddr(node, tc.naples_intf_mac_dict, True, isRollback)
    else:
        result3 = api.types.status.SUCCESS
    if any([result1, result2, result3]) is True:
        api.Logger.error("UC MAC filter : Trigger -> changeMacAddrTrigger failed ", result1, result2, result3, isRollback)
        result = api.types.status.FAILURE

    return result

# Run ping traffic test
def __run_ping_test(req, tc):
    # run traffic between same vlan ifs
    for pair in tc.workload_pairs:
        w1 = pair[0]
        w2 = pair[1]

        if tc.iterators.ipaf == 'ipv6':
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                    (w1.workload_name, w1.ipv6_address, w2.workload_name, w2.ipv6_address)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                    "ping6 -i 0.2 -c 10 %s -I %s" % (w2.ipv6_address, w1.interface))
        elif tc.iterators.ipaf == 'ipv4':
            cmd_cookie = "%s(%s) --> %s(%s)" %\
                    (w1.workload_name, w1.ip_address, w2.workload_name, w2.ip_address)
            api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                    "ping -i 0.2 -c 10 %s" % (w2.ip_address))
        tc.cmd_cookies.append(cmd_cookie)

def do_bsd_lif_resets():
    nodes = api.GetNaplesHostnames()
    naples_node = nodes[0]

    if api.GetNodeOs(naples_node) != utils.OS_TYPE_BSD:
        return 0

    for intf in api.GetNaplesHostInterfaces(naples_node):
        api.Logger.info("LIF reset %s interface %s" %
                        (naples_node, intf))
        status = utils.BsdLifReset(naples_node, intf)
        if status:
            return status
    return 0

def Setup(tc):
    tc.skip = False

    if tc.args.type == 'remote_only':
        tc.workload_pairs = api.GetRemoteWorkloadPairs()
    else:
        tc.skip = True

    if len(tc.workload_pairs) == 0:
        api.Logger.info("Skipping Testcase due to no workload pairs.")
        tc.skip = True

    if api.GetNicMode() == 'hostpin' and tc.iterators.ipaf == 'ipv6':
        api.Logger.info("Skipping Testcase: IPv6 not supported in hostpin mode.")
        tc.skip = True

    InitializeMacConfig(tc)

    return api.types.status.SUCCESS


def Trigger(tc):
    if tc.skip:
        return api.types.status.SUCCESS

    status = api.types.status.SUCCESS
    # clean up resources before run
    subif_utils.clearAll()

    # initialize config:
    subif_utils.initialize_tagged_config_workloads()

    # Delete existing subinterfaces
    __delete_subifs()

    time.sleep(3)
    # Create subinterfaces for every workload/host interface
    # as per <subif_count>
    __create_subifs()

    time.sleep(2)
    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    tc.cmd_cookies = []
    req1 = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    __run_ping_test(req1, tc)
    tc.resp2 = api.Trigger(req1)

    result = changeMacAddrTrigger(tc)
    time.sleep(5)
    api.Logger.debug("UC MAC filter : Trigger -> Change MAC addresses result ", result)

    api.Logger.debug("UC MAC filter : LIF reset ")
    # reset the LIFs if requested
    lif_reset  = getattr(tc.args, 'lif_reset', False)
    if lif_reset:
        if do_bsd_lif_resets():
            api.Logger.error("UC MAC filter : LIF reset failed")
            status = api.types.status.FAILURE

    tc.wload_ep_set, tc.host_ep_set, tc.naples_ep_set, tc.hal_ep_set = ValidateMacRegistration(tc)

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    tc.workload_pairs = api.GetRemoteWorkloadPairs()
    __run_ping_test(req, tc)
    tc.resp = api.Trigger(req)

    return status

def Verify(tc):
    subif_utils.clearAll()
    if tc.skip: return api.types.status.SUCCESS
    if tc.resp2 is None:
        return api.types.status.FAILURE
    if tc.resp is None:
        return api.types.status.FAILURE

    result = api.types.status.SUCCESS
    # Check if MACs in "halctl show endpoint" match with host & workload interface MAC
    if not verifyEndPoints(tc):
        api.Logger.error("UC MAC filter : Verify failed for verifyEndPoints")
        result = api.types.status.FAILURE
    else:
        api.Logger.info("UC MAC filter : Verify - verifyEndPoints SUCCESS ")

    cookie_idx = 0

    for cmd in tc.resp2.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
        cookie_idx += 1

    api.Logger.info("Ping results after MAC Change\n")
    for cmd in tc.resp.commands:
        api.Logger.info("Ping Results for %s" % (tc.cmd_cookies[cookie_idx]))
        api.PrintCommandResults(cmd)
        if cmd.exit_code != 0:
            result = api.types.status.FAILURE
        cookie_idx += 1

    return result

def Teardown(tc):
    result = changeMacAddrTrigger(tc, True)
    api.Logger.debug("UC MAC filter : Teardown -> rolling back MAC address changes ", result)
    return api.types.status.SUCCESS
