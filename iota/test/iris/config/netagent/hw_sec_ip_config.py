#! /usr/bin/python3
import ipaddress
import iota.harness.api as api
import iota.harness.infra.resmgr as resmgr

def ConfigWorkloadSecondaryIp(workload, is_add, sec_ip_count_per_intf=1):
    res = api.types.status.SUCCESS
    wl_sec_ip_list = []
    if (workload.uplink_vlan != 0):
        return wl_sec_ip_list
    
    nodes = api.GetWorkloadNodeHostnames()
    max_untag_wl = 0
    max_tag_wl = 0
    if is_add == True:
        op = "add"
    else:
        op = "del"

    is_wl_bm_type = False
    for node in nodes:
        if api.IsBareMetalWorkloadType(node):
            is_wl_bm_type = True
        workloads = api.GetWorkloads(node)
        num_untag_wl_in_node = 0
        num_tag_wl_in_node = 0
        for wl in workloads:
            if (wl.uplink_vlan == 0):
                num_untag_wl_in_node += 1
            else:
                num_tag_wl_in_node += 1
        if num_untag_wl_in_node > max_untag_wl:
            max_untag_wl = num_untag_wl_in_node
        if num_tag_wl_in_node > max_tag_wl:
            max_tag_wl = num_tag_wl_in_node
        #api.Logger.info("Node {} WL #untag {} #tag {} ".format(node, num_untag_wl_in_node, num_tag_wl_in_node))

    #api.Logger.info("Topo Max untag WL {} Max tag WL {} ".format(max_untag_wl, max_tag_wl))
    sec_ip_incr_step = max_untag_wl
    if is_wl_bm_type == False:
        sec_ip_incr_step += max_tag_wl

    req = api.Trigger_CreateExecuteCommandsRequest(serial = True)

    wl = workload
    sec_ipv4_allocator = resmgr.IpAddressStep(wl.ip_prefix.split('/')[0],
                    str(ipaddress.IPv4Address(sec_ip_incr_step)),sec_ip_count_per_intf)
    sec_ip4_addr_str = str(sec_ipv4_allocator.Alloc())
    for i in range(sec_ip_count_per_intf):
        sec_ip4_addr_str = str(sec_ipv4_allocator.Alloc())
        sec_ip_prefix = sec_ip4_addr_str + "/" + str(wl.ip_prefix.split('/')[1])
        wl_sec_ip_list.append(sec_ip4_addr_str)
        #api.Logger.info("Node {} WL {} Intf {} Secondary IP {} Op {}".format(
        #                 wl.node_name, wl.workload_name, wl.interface, sec_ip_prefix, op))
        api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                           "ip address %s %s dev %s " %(op, sec_ip_prefix, wl.interface))
    trig_resp = api.Trigger(req)

    #api.Logger.info("Response ")
    #for cmd in trig_resp.commands:
    #    api.PrintCommandResults(cmd)

    return wl_sec_ip_list
