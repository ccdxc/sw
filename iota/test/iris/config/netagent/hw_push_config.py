#! /usr/bin/python3
import sys
import pdb
import os
import ipaddress
import time
import copy
from collections import defaultdict

import iota.harness.api as api
import iota.harness.infra.utils.parser as parser
import iota.test.iris.config.netagent.api as netagent_api
import iota.harness.infra.resmgr as resmgr
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

import iota.protos.pygen.topo_svc_pb2 as topo_svc


ipv4_subnet_allocator = {} # network name(config.yaml) --> list of ipv4 addresses
ipv6_subnet_allocator = {} # network name(config.yaml) --> list of ipv6 addresses
classic_mac_allocator = resmgr.MacAddressStep("00AA.0000.0001", "0000.0000.0001")

#This class is responsible for testbed vlan allocation
#It maintain mapping for each config vlan to testbed vlan
class NodeVlan:
    __tb_vlan_map = {}
    __native_vlan = None
    def __init__(self, node_name):
        self.__node_name = node_name
        self.__native_vlan = None
        self.__if_vlan_map = {}
        intf = api.AllocateHostInterfaceForNode(self.__node_name)
        self.__if_vlan_map[intf] = {}

    def __allocateWireVlan(self, intf, encap_vlan):
        wire_vlan = self.__if_vlan_map[intf].get(encap_vlan)
        if wire_vlan:
            return wire_vlan
        #if testbed has mapping
        wire_vlan = NodeVlan.__tb_vlan_map.get(encap_vlan)
        if wire_vlan:
            self.__if_vlan_map[intf][encap_vlan] = wire_vlan
            return wire_vlan
        #If here , need to allocate a testbed vlan
        wire_vlan = api.Testbed_AllocateVlan()
        if not NodeVlan.__native_vlan:
            NodeVlan.__native_vlan = wire_vlan
        self.__tb_vlan_map[encap_vlan] = wire_vlan
        self.__if_vlan_map[intf][encap_vlan] = wire_vlan
        return wire_vlan

    def AllocateHostIntfWireVlan(self, encap_vlan):
        for intf in self.__if_vlan_map:
            wire_vlan = self.__allocateWireVlan(intf, encap_vlan)
            assert (wire_vlan)
            return intf, wire_vlan

    def IsNativeVlan(self, vlan):
        return NodeVlan.__native_vlan == vlan


classic_vlan_map = {}

def __prepare_ip_address_str_for_endpoint(ep):
    nw_filter = "meta.name=" + ep.spec.network_name + ";"
    objects = netagent_api.QueryConfigs(kind='Network', filter=nw_filter)
    assert(len(objects) == 1)
    nw_obj = objects[0]
    ep_spec_ip = ep.spec.ipv4_addresses[0]
    nw_spec_subnet = nw_obj.spec.ipv4_subnet
    ip_str = ep_spec_ip.split('/')[0] + '/' + nw_spec_subnet.split('/')[1]
    return ip_str

def __prepare_ipv6_address_str_for_endpoint(ep):
    nw_filter = "meta.name=" + ep.spec.network_name + ";"
    objects = netagent_api.QueryConfigs(kind='Network', filter=nw_filter)
    assert(len(objects) == 1)
    nw_obj = objects[0]
    ep_spec_ip = ep.spec.ipv6_address
    nw_spec_subnet = nw_obj.spec.ipv6_subnet
    ip_str = ep_spec_ip.split('/')[0] + '/' + nw_spec_subnet.split('/')[1]
    return ip_str

def __get_l2segment_vlan_for_endpoint(ep):
    nw_filter = "meta.name=" + ep.spec.network_name + ";"
    objects = netagent_api.QueryConfigs(kind='Network', filter=nw_filter)
    assert(len(objects) == 1)
    return objects[0].spec.vlan_id

def __add_config_worklads(req, target_node = None):
    third_party_workload_count = 0
    ep_objs = netagent_api.QueryConfigs(kind='Endpoint')
    for ep in ep_objs:
        node_name = getattr(ep.spec, "_node_name", None)
        if not node_name:
            node_name = ep.spec.node_uuid
        if target_node and target_node != node_name:
            api.Logger.debug("Skipping add workload for node %s" % node_name)
            continue
        if not api.IsNaplesNode(node_name):
            #if api.GetNicMode() == 'hostpin' and third_party_workload_count > 0:
            #    continue
            third_party_workload_count += 1
        req.workload_op = topo_svc.ADD
        wl_msg = req.workloads.add()
        wl_msg.workload_name = ep.meta.name
        wl_msg.node_name = node_name
        wl_msg.ip_prefix = __prepare_ip_address_str_for_endpoint(ep)
        #wl_msg.ipv6_prefix = __prepare_ipv6_address_str_for_endpoint(ep)
        wl_msg.mac_address = ep.spec.mac_address
        wl_msg.pinned_port = 1
        wl_msg.interface_type = topo_svc.INTERFACE_TYPE_VSS
        encap_vlan = getattr(ep.spec, 'useg_vlan', None)
        host_if = None
        if api.GetNicMode() == 'hostpin':
            host_if = api.AllocateHostInterfaceForNode(wl_msg.node_name)
            wl_msg.uplink_vlan = __get_l2segment_vlan_for_endpoint(ep)
            if api.GetNicType(wl_msg.node_name) in ['pensando', 'naples']:
                wl_msg.encap_vlan = encap_vlan if encap_vlan else wl_msg.uplink_vlan
            else:
                wl_msg.encap_vlan = wl_msg.uplink_vlan
        elif api.GetNicMode() == 'classic':
            global classic_vlan_map
            node_vlan = classic_vlan_map.get(node_name)
            if not node_vlan:
                node_vlan = NodeVlan(node_name)
                classic_vlan_map[node_name] = node_vlan
            #Allocate only if useg_vlan present.
            host_if, wire_vlan = node_vlan.AllocateHostIntfWireVlan(encap_vlan)
            if not node_vlan.IsNativeVlan(wire_vlan):
                #Set encap vlan if its non native.
                wl_msg.encap_vlan = wire_vlan
                wl_msg.uplink_vlan = wire_vlan
        else:
            assert(0)
        wl_msg.interface = host_if
        wl_msg.parent_interface = host_if

        wl_msg.workload_type = api.GetWorkloadTypeForNode(wl_msg.node_name)
        wl_msg.workload_image = api.GetWorkloadImageForNode(wl_msg.node_name)
        wl_msg.cpus = api.GetWorkloadCpusForNode(wl_msg.node_name)
        wl_msg.memory = api.GetWorkloadMemoryForNode(wl_msg.node_name)

def GetIPv4Allocator(nw_name):
    global ipv4_subnet_allocator
    if nw_name in ipv4_subnet_allocator:
        subnet = copy.deepcopy(ipv4_subnet_allocator[nw_name])
        return subnet
    else:
        return None

def GetIPv6Allocator(nw_name):
    global ipv6_subnet_allocator
    if nw_name in ipv6_subnet_allocator:
        subnet = copy.deepcopy(ipv6_subnet_allocator[nw_name])
        return subnet
    else:
        return None

def GetMacAllocator():
    return copy.deepcopy(classic_mac_allocator)

def __add_config_classic_workloads(req, target_node = None):
    classic_yml = "{}/config.yml".format(api.GetTopologyDirectory())
    spec = parser.YmlParse(classic_yml)
    api.Logger.info("Config yml: \n %s" % classic_yml)
    req.workload_op = topo_svc.ADD
    # Saving node IFs
    nodes = api.GetWorkloadNodeHostnames()
    node_ifs = {}
    for node in nodes:
        if target_node != None and node != target_node:
            continue
        node_ifs[node] = api.GetNaplesHostInterfaces(node)
    # Reading network and workload specs
    network_specs = {}
    workload_specs = {}
    for net in spec.spec.networks:
        network_specs[net.network.name] = net.network
    for wl in spec.spec.workloads:
        workload_specs[wl.workload.name] = wl.workload
    # Forming subnet allocators
    subnet_allocator = {}
    sec_ipv4_subnet_allocator = {}
    sec_ipv6_subnet_allocator = {}
    global ipv4_subnet_allocator
    ipv4_subnet_allocator.clear()

    global ipv6_subnet_allocator
    ipv6_subnet_allocator.clear()

    global classic_mac_allocator

    for k,net in network_specs.items():
        if net.ipv4.enable:
            ipv4_subnet_allocator[net.name] = resmgr.IpAddressStep(net.ipv4.ipam_base.split('/')[0], # 10.255.0.0/16 \
                                                                   str(ipaddress.IPv4Address(1 << int(net.ipv4.ipam_base.split('/')[1]))))
            sec_ipv4_subnet_allocator[net.name] = []
            for ipb in getattr(net.ipv4, "secondary_ipam_base", []):
                s = ipaddress.IPv4Network(ipb)
                sec_ipv4_subnet_allocator[net.name].append(s.subnets(new_prefix=net.ipv4.prefix_length))

        if net.ipv6.enable:
            ipv6_subnet_allocator[net.name] = resmgr.Ipv6AddressStep(net.ipv6.ipam_base.split('/')[0], # 2000::/48 \
                                                                   str(ipaddress.IPv6Address(1 << int(net.ipv6.ipam_base.split('/')[1]))))
            sec_ipv6_subnet_allocator[net.name] = []
            for ipb in getattr(net.ipv6, "secondary_ipam_base", []):
                s = ipaddress.IPv6Network(ipb)
                sec_ipv6_subnet_allocator[net.name].append(s.subnets(new_prefix=net.ipv6.prefix_length))

    classic_mac_allocator = resmgr.MacAddressStep("00AA.0000.0001", "0000.0000.0001")
    # Using up first vlan for native
    native_vlan = api.Testbed_AllocateVlan()
    # Forming native workloads
    native_nw_spec = {} # intf -> nw_spec
    native_ipv4_allocator = {} # intf -> ipv4_allocator
    native_ipv6_allocator = {} # intf -> ipv6_allocator
    for workload in spec.instances.workloads:
        wl = workload.workload
        if wl.spec == 'native':
            for intf in wl.interfaces:
                if intf not in native_nw_spec:
                    wl_spec = workload_specs[wl.spec]
                    nw_spec = network_specs[wl_spec.network_spec]
                    native_nw_spec[intf] = nw_spec
                    ipv4_subnet = ipv4_subnet_allocator[nw_spec.name].Alloc()
                    native_ipv4_allocator[intf] = resmgr.IpAddressStep(ipv4_subnet, "0.0.0.1")
                    native_ipv4_allocator[intf].Alloc() # To skip 0 ip
                    ipv6_subnet = ipv6_subnet_allocator[nw_spec.name].Alloc()
                    native_ipv6_allocator[intf] = resmgr.Ipv6AddressStep(ipv6_subnet, "0::1")
                    native_ipv6_allocator[intf].Alloc() # To skip 0 ip
                nw_spec = native_nw_spec[intf]
                ipv4_allocator = native_ipv4_allocator[intf]
                ipv6_allocator = native_ipv6_allocator[intf]
                vlan = 0
                node_intf = node_ifs[wl.node][int(intf.replace('host_if', '')) - 1]
                wl_msg = req.workloads.add()
                ip4_addr_str = str(ipv4_allocator.Alloc())
                ip6_addr_str = str(ipv6_allocator.Alloc())
                wl_msg.ip_prefix = ip4_addr_str + "/" + str(nw_spec.ipv4.prefix_length)
                wl_msg.ipv6_prefix = ip6_addr_str + "/" + str(nw_spec.ipv6.prefix_length)
                wl_msg.mac_address = classic_mac_allocator.Alloc().get()
                wl_msg.encap_vlan = vlan
                wl_msg.uplink_vlan = wl_msg.encap_vlan
                wl_msg.workload_name = wl.node + "_" + node_intf + "_subif_" + str(vlan)
                wl_msg.node_name = wl.node
                wl_msg.pinned_port = 1
                wl_msg.interface_type = topo_svc.INTERFACE_TYPE_VSS
                wl_msg.interface = node_intf
                wl_msg.parent_interface = node_intf
                wl_msg.workload_type = api.GetWorkloadTypeForNode(wl.node)
                wl_msg.workload_image = api.GetWorkloadImageForNode(wl.node)
    # Forming subif  workloads
    nw_specs = {} # ith subif -> nw_spec
    ipv4_allocators = {} # ith subif -> ipv4_allocator
    ipv6_allocators = {} # ith subif  -> ipv6_allocator
    sec_ipv4_allocators = {} # ith subif -> ipv4_allocator
    sec_ipv6_allocators = {} # ith subif  -> ipv6_allocator
    tagged_vlan = api.Testbed_AllocateVlan()
    vlans = {} # ith subif -> vlan
    for workload in spec.instances.workloads:
        wl = workload.workload
        if wl.spec == 'tagged':
            if wl.count == 'auto':
                # Uncomment this once the bug PS-724 is fixed
                #num_subifs = (api.Testbed_GetVlanCount() - 1) # 1 for native
                num_subifs = 32
            else:
                num_subifs = int(wl.count)
            for i in range(num_subifs):
                if i not in nw_specs:
                    wl_spec = workload_specs[wl.spec]
                    nw_spec = network_specs[wl_spec.network_spec]
                    nw_specs[i] = nw_spec
                    ipv4_subnet = ipv4_subnet_allocator[nw_spec.name].Alloc()
                    ipv4_allocators[i] = resmgr.IpAddressStep(ipv4_subnet, "0.0.0.1")
                    ipv4_allocators[i].Alloc() # To skip 0 ip

                    ipv6_subnet = ipv6_subnet_allocator[nw_spec.name].Alloc()
                    ipv6_allocators[i] = resmgr.Ipv6AddressStep(ipv6_subnet, "0::1")
                    ipv6_allocators[i].Alloc() # To skip 0 ip

                    sec_ipv4_allocators[i] = []
                    for s in sec_ipv4_subnet_allocator[nw_spec.name]:
                        sec_ipv4_subnet = next(s)
                        sec_ipv4_allocators[i].append(sec_ipv4_subnet.hosts())

                    sec_ipv6_allocators[i] = []
                    for s in sec_ipv6_subnet_allocator[nw_spec.name]:
                        sec_ipv6_subnet = next(s)
                        sec_ipv6_allocators[i].append(sec_ipv6_subnet.hosts())

                    if api.GetWorkloadTypeForNode(wl.node) == topo_svc.WorkloadType.Value('WORKLOAD_TYPE_BARE_METAL'):
                        vlans[i] = api.Testbed_AllocateVlan()
                    else:
                        vlans[i] = tagged_vlan
                intf = wl.interfaces[i % len(wl.interfaces)]
                nw_spec = nw_specs[i]
                ipv4_allocator = ipv4_allocators[i]
                ipv6_allocator = ipv6_allocators[i]
                vlan = vlans[i]
                node_intf = node_ifs[wl.node][int(intf.replace('host_if', '')) - 1]
                wl_msg = req.workloads.add()
                ip4_addr_str = str(ipv4_allocator.Alloc())
                ip6_addr_str = str(ipv6_allocator.Alloc())
                wl_msg.ip_prefix = ip4_addr_str + "/" + str(nw_spec.ipv4.prefix_length)
                wl_msg.ipv6_prefix = ip6_addr_str + "/" + str(nw_spec.ipv6.prefix_length)
                wl_msg.mac_address = classic_mac_allocator.Alloc().get()
                wl_msg.encap_vlan = vlan
                wl_msg.uplink_vlan = wl_msg.encap_vlan
                wl_msg.node_name = wl.node
                wl_msg.pinned_port = 1
                wl_msg.interface_type = topo_svc.INTERFACE_TYPE_VSS
                if api.GetWorkloadTypeForNode(wl.node) == topo_svc.WorkloadType.Value('WORKLOAD_TYPE_BARE_METAL'):
                    wl_msg.interface = node_intf + "_" + str(vlan)
                    wl_msg.workload_name = wl.node + "_" + node_intf + "_subif_" + str(vlan)
                else:
                    wl_msg.interface = node_intf + "_mv%d" % (i+1)
                    wl_msg.workload_name = wl.node + "_" + node_intf + "_mv%d" % (i+1)
                #wl_msg.interface = node_intf
                wl_msg.parent_interface = node_intf
                wl_msg.workload_type = api.GetWorkloadTypeForNode(wl.node)
                wl_msg.workload_image = api.GetWorkloadImageForNode(wl.node)
                for a in sec_ipv4_allocators[i]:
                    wl_msg.sec_ip_prefix.append(str(next(a))+"/"+str(nw_spec.ipv4.prefix_length))
                for a in sec_ipv6_allocators[i]:
                    wl_msg.sec_ipv6_prefix.append(str(next(a))+"/"+str(nw_spec.ipv6.prefix_length))

'''
def __add_config_classic_workloads(req, target_node = None):
    classic_yml = "{}/classic.yml".format(api.GetTopologyDirectory())
    spec = parser.YmlParse(classic_yml)
    spec.subifs.num = getattr(spec.subifs, 'num', 0)
    api.Logger.info("No of Subif Workloads to be created: %d" % spec.subifs.num)
    # Note: Make sure the num in yml should always
    #       be less than TESTBED_NUM_VLANS_CLASSIC
    req.workload_op = topo_svc.ADD
    nodes = api.GetWorkloadNodeHostnames()
    subnet_allocator = resmgr.IpAddressStep("192.170.1.1", "0.0.1.0")
    ipv6_subnet_allocator = resmgr.Ipv6AddressStep("1000::0:1", "0::1:0:0")
    mac_allocator = resmgr.MacAddressStep("00AA.0000.0001", "0000.0000.0001")
    api.Testbed_AllocateVlan()

    # For every subif, create 1 WL on each node
    # Get subnet of this subif
    num_ifs = 0
    node_ifs = {}
    for node in nodes:
        if target_node != None and node != target_node:
            continue
        node_ifs[node] = api.GetNaplesHostInterfaces(node)
        if num_ifs == 0:
            num_ifs = len(node_ifs[node])
        num_ifs = min(len(node_ifs[node]), num_ifs)

    # Hardcoding this for now.
    num_ifs = 2
    vlan = 0
    for i in range(num_ifs):
        subnet = subnet_allocator.Alloc()
        ipv6_subnet = ipv6_subnet_allocator.Alloc()
        ip4_allocator = resmgr.IpAddressStep(subnet, "0.0.0.1")
        ip6_allocator = resmgr.Ipv6AddressStep(ipv6_subnet, "0::1")
        for node in nodes:
            wl_msg = req.workloads.add()
            ip4_addr_str = str(ip4_allocator.Alloc())
            ip6_addr_str = str(ip6_allocator.Alloc())
            wl_msg.ip_prefix = ip4_addr_str + "/24"
            wl_msg.ipv6_prefix = ip6_addr_str + "/96"
            wl_msg.mac_address = mac_allocator.Alloc().get()
            wl_msg.encap_vlan = vlan
            wl_msg.uplink_vlan = wl_msg.encap_vlan
            wl_msg.workload_name = node + "_" + node_ifs[node][i] + "_subif_" + str(vlan)
            wl_msg.node_name = node
            wl_msg.pinned_port = 1
            wl_msg.interface_type = topo_svc.INTERFACE_TYPE_VSS
            wl_msg.interface = node_ifs[node][i]
            wl_msg.parent_interface = node_ifs[node][i]
            wl_msg.workload_type = api.GetWorkloadTypeForNode(node)
            wl_msg.workload_image = api.GetWorkloadImageForNode(node)

    # Subif_num -> vlan, ipv4 address, ipv6 address
    #vlans = {}
    #ip4_alloc = {}
    #ip6_alloc = {}
    for i in range(num_ifs):
        for j in range(spec.subifs.num):
            subnet = subnet_allocator.Alloc()
            ipv6_subnet = ipv6_subnet_allocator.Alloc()
            vlan = api.Testbed_AllocateVlan()
            ip4_alloc = resmgr.IpAddressStep(subnet, "0.0.0.1")
            ip6_alloc = resmgr.Ipv6AddressStep(ipv6_subnet, "0::1")
            for node in nodes:
               if target_node != None and node != target_node:
                   continue
               #pdb.set_trace()
               wl_msg = req.workloads.add()
               ip4_addr_str = str(ip4_alloc.Alloc())
               ip6_addr_str = str(ip6_alloc.Alloc())
               wl_msg.ip_prefix = ip4_addr_str + "/24"
               wl_msg.ipv6_prefix = ip6_addr_str + "/96"
               wl_msg.mac_address = mac_allocator.Alloc().get()
               wl_msg.encap_vlan = vlan
               wl_msg.uplink_vlan = wl_msg.encap_vlan
               wl_msg.workload_name = node + "_" + node_ifs[node][i] + "_subif_" + str(vlan)
               wl_msg.node_name = node
               wl_msg.pinned_port = 1
               wl_msg.interface_type = topo_svc.INTERFACE_TYPE_VSS
               wl_msg.interface = node_ifs[node][i]
               wl_msg.parent_interface = node_ifs[node][i]
               wl_msg.workload_type = api.GetWorkloadTypeForNode(node)
               wl_msg.workload_image = api.GetWorkloadImageForNode(node)
'''

def __add_workloads(target_node = None):
    req = topo_svc.WorkloadMsg()
    if api.GetNicMode() == 'hostpin':
        __add_config_worklads(req, target_node)
    elif api.GetNicMode() == 'classic':
        __add_config_classic_workloads(req, target_node)
    else:
        assert(0)

    if len(req.workloads):
        resp = api.AddWorkloads(req, skip_bringup=api.IsConfigOnly())
        if resp is None:
            sys.exit(1)

def __recover_workloads(target_node = None):
    req = topo_svc.WorkloadMsg()
    resp = api.RestoreWorkloads(req)
    if resp is None:
        sys.exit(1)

def AddWorkloads():
    __add_workloads()

def RestoreWorkloads():
    __recover_workloads()

def AddNaplesWorkloads(target_node=None):
    req = topo_svc.WorkloadMsg()
    req.workload_op = topo_svc.ADD
    wloads = api.GetNaplesWorkloads()
    for wl in wloads:
        if wl.SkipNodePush() or (target_node and target_node != wl.node_name):
            api.Logger.debug("Skipping add workload for node %s" % wl.node_name)
            continue
        wl_msg = req.workloads.add()
        wl_msg.workload_name = wl.workload_name
        wl_msg.node_name = wl.node_name
        wl_msg.ip_prefix = wl.ip_address + "/" + '24'
        #wl_msg.ipv6_prefix = __prepare_ipv6_address_str_for_endpoint(ep)
        wl_msg.interface = wl.interface
        wl_msg.parent_interface = wl.interface
        wl_msg.interface_type = topo_svc.INTERFACE_TYPE_VSS
        wl_msg.workload_type  = topo_svc.WorkloadType.Value('WORKLOAD_TYPE_BARE_METAL')
    if len(req.workloads):
        resp = api.AddWorkloads(req, skip_store=True)
        if resp is None:
            sys.exit(1)


def __delete_classic_workloads(target_node = None):
    req = topo_svc.WorkloadMsg()
    for wl in api.GetWorkloads():
        if target_node and target_node != wl.node_name:
            api.Logger.debug("Skipping delete workload for node %s" % wl.node_name)
            continue
        req.workload_op = topo_svc.DELETE
        wl_msg = req.workloads.add()
        wl_msg.workload_name = wl.workload_name
        wl_msg.node_name = wl.node_name
    if len(req.workloads):
        resp = api.DeleteWorkloads(req, True)
        if resp is None:
            sys.exit(1)

def __readd_classic_workloads(target_node = None):
    req = topo_svc.WorkloadMsg()
    req.workload_op = topo_svc.ADD
    #pdb.set_trace()
    for wl in api.GetWorkloads():
        if target_node and target_node != wl.node_name:
            api.Logger.debug("Skipping add classic workload for node %s" % wl.node_name)
            continue
        wl_msg = req.workloads.add()
        wl_msg.ip_prefix = wl.ip_prefix
        wl_msg.ipv6_prefix = wl.ipv6_prefix
        wl_msg.mac_address = wl.mac_address
        wl_msg.encap_vlan = wl.encap_vlan
        wl_msg.uplink_vlan = wl.uplink_vlan
        wl_msg.workload_name = wl.workload_name
        wl_msg.node_name = wl.node_name
        wl_msg.pinned_port = wl.pinned_port
        wl_msg.interface_type = wl.interface_type
        wl_msg.interface = wl.parent_interface
        wl_msg.parent_interface = wl.parent_interface
        wl_msg.workload_type = wl.workload_type
        wl_msg.workload_image = wl.workload_image
    if len(req.workloads):
        resp = api.AddWorkloads(req)
        if resp is None:
            sys.exit(1)

def __delete_workloads(target_node = None):
    ep_objs = netagent_api.QueryConfigs(kind='Endpoint')
    req = topo_svc.WorkloadMsg()
    for ep in ep_objs:
        node_name = getattr(ep.spec, "_node_name", None)
        if not node_name:
            node_name = ep.spec.node_uuid
        if target_node and target_node != node_name:
            api.Logger.info("Skipping delete workload for node %s" % node_name)
            continue
        req.workload_op = topo_svc.DELETE
        wl_msg = req.workloads.add()
        wl_msg.workload_name = ep.meta.name
        wl_msg.node_name = node_name

    if len(req.workloads):
        resp = api.DeleteWorkloads(req)
        if resp is None:
            sys.exit(1)

def ReAddWorkloads(node):
    if api.GetNicMode() == 'classic':
        __delete_classic_workloads(node)
        __readd_classic_workloads(node)
    else:
        __delete_workloads(node)
        __add_workloads(node)
    AddNaplesWorkloads(node)



def Main(args):
    #time.sleep(120)
    agent_nodes = api.GetNaplesHostnames()
    netagent_api.Init(agent_nodes, hw = True)

    netagent_api.ReadConfigs(api.GetTopologyDirectory())
    #Delete path is not stable yet
    #netagent_api.DeleteBaseConfig()

    if GlobalOptions.skip_setup:
        RestoreWorkloads()
    else:
        if api.GetNicMode() != 'classic':
            netagent_api.PushBaseConfig()
        __add_workloads()
    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
