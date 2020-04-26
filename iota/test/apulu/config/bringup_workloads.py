#! /usr/bin/python3
import sys

import iota.harness.api as api
import iota.harness.infra.resmgr as resmgr

import iota.test.apulu.config.api as config_api

import iota.protos.pygen.topo_svc_pb2 as topo_svc

__max_udp_ports = 1
__max_tcp_ports = 1

portUdpAllocator = resmgr.TestbedPortAllocator(205)
portTcpAllocator = resmgr.TestbedPortAllocator(4500)

def __add_secondary_ip_to_workloads(workloads=[]):
    if not api.IsSimulation():
        req = api.Trigger_CreateAllParallelCommandsRequest()
    else:
        req = api.Trigger_CreateExecuteCommandsRequest(serial = False)

    workloads = workloads if workloads else api.GetWorkloads()
    for wl in workloads:
        for sec_ip_addr in wl.sec_ip_addresses:
            api.Trigger_AddCommand(req, wl.node_name, wl.workload_name,
                                   "ifconfig %s add %s" % (wl.interface, sec_ip_addr))
            api.Logger.debug("ifconfig add from %s %s %s %s" % (wl.node_name, wl.workload_name, wl.interface, sec_ip_addr))

    resp = api.Trigger(req)
    if resp is None:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS

def _add_exposed_ports(wl_msg):
    if  wl_msg.workload_type != topo_svc.WORKLOAD_TYPE_CONTAINER:
        return
    for p in ["4500", "4501", "4507"]:
        tcp_port = wl_msg.exposed_ports.add()
        tcp_port.Port = p
        tcp_port.Proto = "tcp"

    for _ in range(__max_udp_ports):
        udp_port = wl_msg.exposed_ports.add()
        udp_port.Port = "1001"
        udp_port.Proto = "udp"


def __add_workloads(redirect_port):

    req = topo_svc.WorkloadMsg()
    req.workload_op = topo_svc.ADD

    for ep in config_api.GetEndpoints():
        wl_msg = req.workloads.add()
        # Make the workload_name unique across nodes by appending node-name
        wl_msg.workload_name = ep.name + ep.node_name
        wl_msg.node_name = ep.node_name
        wl_msg.ip_prefix = ep.ip_addresses[0]
        wl_msg.sec_ip_prefix.extend(ep.ip_addresses[1:])
        # wl_msg.ipv6_prefix = ep.ip_addresses[1]
        wl_msg.mac_address = ep.macaddr
        wl_msg.interface_type = topo_svc.INTERFACE_TYPE_NONE
        wl_msg.encap_vlan = ep.vlan
        interface = ep.interface
        if interface != None: wl_msg.interface = interface
        wl_msg.parent_interface = wl_msg.interface
        wl_msg.workload_type = api.GetWorkloadTypeForNode(wl_msg.node_name)
        wl_msg.workload_image = api.GetWorkloadImageForNode(wl_msg.node_name)
        wl_msg.mgmt_ip = api.GetMgmtIPAddress(wl_msg.node_name)
        if redirect_port:
            _add_exposed_ports(wl_msg)
        api.Logger.info(f"Workload {wl_msg.workload_name} "
                        f"Node {wl_msg.node_name} Intf {wl_msg.interface} "
                        f"IP {wl_msg.ip_prefix} MAC {wl_msg.mac_address}"
                        f"VLAN {wl_msg.encap_vlan}")

    if len(req.workloads):
        api.Logger.info("Adding %d Workloads" % len(req.workloads))
        resp = api.AddWorkloads(req, skip_bringup=api.IsConfigOnly())
        if resp is None:
            sys.exit(1)

        for ep in config_api.GetEndpoints():
            workload_name = ep.name + ep.node_name
            for wl in api.GetWorkloads():
                if wl.workload_name == workload_name:
                    wl.vnic = ep.vnic

def __delete_classic_workloads(target_node = None, workloads = None):

    req = topo_svc.WorkloadMsg()
    req.workload_op = topo_svc.DELETE

    workloads = workloads if workloads else api.GetWorkloads()
    for wl in workloads:
        if target_node and target_node != wl.node_name:
            api.Logger.debug("Skipping delete workload for node %s" % wl.node_name)
            continue

        wl_msg = req.workloads.add()
        wl_msg.workload_name = wl.workload_name
        wl_msg.node_name = wl.node_name

    if len(req.workloads):
        resp = api.DeleteWorkloads(req, skip_store=True)
        if resp is None:
            sys.exit(1)

def __readd_classic_workloads(target_node = None, workloads = []):

    req = topo_svc.WorkloadMsg()
    req.workload_op = topo_svc.ADD

    workloads = workloads if workloads else api.GetWorkloads()
    for wl in workloads:
        if target_node and target_node != wl.node_name:
            api.Logger.debug("Skipping add classic workload for node %s" % wl.node_name)
            continue

        wl_msg = req.workloads.add()
        wl_msg.ip_prefix = wl.ip_prefix
        wl_msg.ipv6_prefix = wl.ipv6_prefix
        wl_msg.sec_ip_prefix.extend(wl.sec_ip_prefixes)
        wl_msg.mac_address = wl.mac_address
        wl_msg.encap_vlan = wl.encap_vlan
        wl_msg.uplink_vlan = wl.uplink_vlan
        wl_msg.workload_name = wl.workload_name
        wl_msg.node_name = wl.node_name
        wl_msg.pinned_port = wl.pinned_port
        wl_msg.interface_type = wl.interface_type
        interface = wl.interface
        if interface != None: wl_msg.interface = interface
        wl_msg.parent_interface = wl_msg.interface
        wl_msg.workload_type = wl.workload_type
        wl_msg.workload_image = wl.workload_image
        wl_msg.mgmt_ip = api.GetMgmtIPAddress(wl_msg.node_name)

    if len(req.workloads):
        resp = api.AddWorkloads(req, skip_store=True)
        if resp is None:
            sys.exit(1)

def ReAddWorkloads(node):
    __delete_classic_workloads(node)
    __readd_classic_workloads(node)

def DeleteWorkload(wl):
    __delete_classic_workloads(workloads=[wl])

def ReAddWorkload(wl):
    __readd_classic_workloads(workloads=[wl])
    __add_secondary_ip_to_workloads([wl])

def Main(args):
    api.Logger.info("Adding Workloads")
    if args != None and hasattr(args, 'trex'):
        redirect_port = args.trex
    else:
        redirect_port = False
    __add_workloads(redirect_port)
    __add_secondary_ip_to_workloads()
    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
