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

class _TopoWorkloadConfig(object):

# ipv4_subnet_allocator = {} # network name(config.yaml) --> list of ipv4 addresses
# ipv6_subnet_allocator = {} # network name(config.yaml) --> list of ipv6 addresses

    def __init__(self):
        # Reading network and workload specs
        self.__network_specs = {}
        self.__workload_specs = {}
        self.__ipv4_subnet_allocator = {}
        self.__ipv6_subnet_allocator = {}
        self.__classic_mac_allocator = None
        self.__subnet_allocator = {}
        self.__sec_ipv4_subnet_allocator = {}
        self.__sec_ipv6_subnet_allocator = {}

        try:
            self.__read_spec()
            self.__init_allocators()
        except Exception as e:
            api.Logger.error("Failed to initialize from config.yml")
        return

    def __init_allocators(self):
        # Forming subnet allocators
        self.__subnet_allocator = {}
        self.__sec_ipv4_subnet_allocator = {}
        self.__sec_ipv6_subnet_allocator = {}
        # global ipv4_subnet_allocator
        self.__ipv4_subnet_allocator.clear()

        # global ipv6_subnet_allocator
        self.__ipv6_subnet_allocator.clear()

        for k,net in self.__network_specs.items():
            if net.ipv4.enable:
                self.__ipv4_subnet_allocator[net.name] = resmgr.IpAddressStep(
                        net.ipv4.ipam_base.split('/')[0], # 10.255.0.0/16 \ 
                        str(ipaddress.IPv4Address(1 << int(net.ipv4.ipam_base.split('/')[1]))))
                self.__sec_ipv4_subnet_allocator[net.name] = []
                for ipb in getattr(net.ipv4, "secondary_ipam_base", []):
                    s = ipaddress.IPv4Network(ipb)
                    self.__sec_ipv4_subnet_allocator[net.name].append(s.subnets(new_prefix=net.ipv4.prefix_length))

            if net.ipv6.enable:
                self.__ipv6_subnet_allocator[net.name] = resmgr.Ipv6AddressStep(
                        net.ipv6.ipam_base.split('/')[0], # 2000::/48 \ 
                        str(ipaddress.IPv6Address(1 << int(net.ipv6.ipam_base.split('/')[1]))))
                self.__sec_ipv6_subnet_allocator[net.name] = []
                for ipb in getattr(net.ipv6, "secondary_ipam_base", []):
                    s = ipaddress.IPv6Network(ipb)
                    self.__sec_ipv6_subnet_allocator[net.name].append(s.subnets(new_prefix=net.ipv6.prefix_length))
        self.__classic_mac_allocator = resmgr.MacAddressStep("00AA.0000.0001", "0000.0000.0001")

    def __read_spec(self):
        classic_yml = "{}/config.yml".format(api.GetTopologyDirectory())
        api.Logger.info("Config yml: \n %s" % classic_yml)

        self.__spec = parser.YmlParse(classic_yml)

        for net in self.__spec.spec.networks:
            self.__network_specs[net.network.name] = net.network
        for wl in self.__spec.spec.workloads:
            self.__workload_specs[wl.workload.name] = wl.workload
        return

    def GetNativeWorkloadInstances(self):
        return list(filter(lambda x: x.workload.spec == 'native', self.__spec.instances.workloads))

    def GetTaggedWorkloadInstances(self):
        return list(filter(lambda x: x.workload.spec == 'tagged', self.__spec.instances.workloads))

    def GetWorkloadSpecByName(self, name):
        return self.__workload_specs.get(name, None)

    def GetNetworkSpecByName(self, name):
        return self.__network_specs.get(name, None)

    def AllocIpv4SubnetAllocator(self, nw_spec_name):
        ipv4_subnet = self.__ipv4_subnet_allocator.get(nw_spec_name, None).Alloc()
        return resmgr.IpAddressStep(ipv4_subnet, "0.0.0.1")

    def AllocIpv6SubnetAllocator(self, nw_spec_name):
        ipv6_subnet = self.__ipv6_subnet_allocator.get(nw_spec_name, None).Alloc()
        return resmgr.Ipv6AddressStep(ipv6_subnet, "0::1")

    def AllocSecondaryIPv4Allocators(self, nw_spec_name):
        v4_allocs = []
        for s in self.__sec_ipv4_subnet_allocator[nw_spec_name]:
            sec_ipv4_subnet = next(s)
            v4_allocs.append(sec_ipv4_subnet.hosts())
        return v4_allocs

    def AllocSecondaryIPv6Allocators(self, nw_spec_name):
        v6_allocs = []
        for s in self.__sec_ipv6_subnet_allocator[nw_spec_name]:
            sec_ipv6_subnet = next(s)
            v6_allocs.append(sec_ipv6_subnet.hosts())
        return v6_allocs

    def GetClassicMacAllocator(self):
        return self.__classic_mac_allocator


TopoWorkloadConfig = _TopoWorkloadConfig()

class SubInterfaceWorkload(object):

    #nw_specs = {} # ith subif -> nw_spec
    #ipv4_allocators = {} # ith subif -> ipv4_allocator
    #ipv6_allocators = {} # ith subif  -> ipv6_allocator
    #sec_ipv4_allocators = {} # ith subif -> ipv4_allocator
    #sec_ipv6_allocators = {} # ith subif  -> ipv6_allocator
    Store = {}

    def __new__(cls, nic_hint, ifindex, wlDefn, default_vlan, allocate_vlan):
        key = '%d' % ifindex
        if key in cls.Store:
            return cls.Store[key]

        # Create & Initialize the instance
        inst = super(SubInterfaceWorkload, cls).__new__(cls)
        cls.Store[key] = inst

        inst.__NicHint = nic_hint
        inst.__IfIndex = ifindex
        if allocate_vlan:
            inst.__vlan = api.Testbed_AllocateVlan()
        else:
            inst.__vlan = default_vlan
        wl_spec = TopoWorkloadConfig.GetWorkloadSpecByName(wlDefn)
        inst.__NetworkSpec = TopoWorkloadConfig.GetNetworkSpecByName(wl_spec.network_spec)
        inst.__PrimaryIpv4Allocator = TopoWorkloadConfig.AllocIpv4SubnetAllocator(inst.__NetworkSpec.name)
        inst.__PrimaryIpv4Allocator.Alloc() # To skip 0 ip

        inst.__PrimaryIpv6Allocator = TopoWorkloadConfig.AllocIpv6SubnetAllocator(inst.__NetworkSpec.name)
        inst.__PrimaryIpv6Allocator.Alloc() # To skip 0 ip
        inst.__SeconadryIpv4Allocator = TopoWorkloadConfig.AllocSecondaryIPv4Allocators(inst.__NetworkSpec.name)
        inst.__SecondaryIpv6Allocator = TopoWorkloadConfig.AllocSecondaryIPv6Allocators(inst.__NetworkSpec.name)
        return inst

    def __init__(self, nic_hint, ifindex, wlDefn, vlan, allocate_vlan):
        pass

    def GetIfIndex(self):
        return self.__IfIndex

    def GetNetworkSpec(self):
        return self.__NetworkSpec

    def GetPrimaryIPv4Allocator(self):
        return self.__PrimaryIpv4Allocator

    def GetPrimaryIPv6Allocator(self):
        return self.__PrimaryIpv6Allocator

    def GetSecondaryIpv4Allocator(self):
        return self.__SeconadryIpv4Allocator

    def GetSecondaryIpv6Allocator(self):
        return self.__SecondaryIpv6Allocator

    def GetVLAN(self):
        return self.__vlan

    def GetIPv4PrefixLength(self):
        return self.__NetworkSpec.ipv4.prefix_length

    def GetIPv6PrefixLength(self):
        return self.__NetworkSpec.ipv6.prefix_length

    def GetClassicMacAllocator(self):
        return TopoWorkloadConfig.GetClassicMacAllocator()


class NativeInterfaceWorkload:

    # native_nw_spec = {} # intf -> nw_spec
    # native_ipv4_allocator = {} # intf -> ipv4_allocator
    # native_ipv6_allocator = {} # intf -> ipv6_allocator

#            if intf not in native_nw_spec:
#                wl_spec = workload_specs[wl.spec]
#                nw_spec = network_specs[wl_spec.network_spec]
#                native_nw_spec[intf] = nw_spec
#                ipv4_subnet = ipv4_subnet_allocator[nw_spec.name].Alloc()
#                native_ipv4_allocator[intf] = resmgr.IpAddressStep(ipv4_subnet, "0.0.0.1")
#                native_ipv4_allocator[intf].Alloc() # To skip 0 ip
#                ipv6_subnet = ipv6_subnet_allocator[nw_spec.name].Alloc()
#                native_ipv6_allocator[intf] = resmgr.Ipv6AddressStep(ipv6_subnet, "0::1")
#                native_ipv6_allocator[intf].Alloc() # To skip 0 ip
#
    Store = {}

    def __new__(cls, nic_hint, ifname, wlDefn):
        key = '%s' % ifname
        if key in cls.Store:
            return cls.Store[key]

        # Create & Initialize the instanec
        inst = super(NativeInterfaceWorkload, cls).__new__(cls)
        cls.Store[key] = inst

        inst.__NicHint = nic_hint
        inst.__ifName = ifname
        wl_spec = TopoWorkloadConfig.GetWorkloadSpecByName(wlDefn)
        inst.__NetworkSpec = TopoWorkloadConfig.GetNetworkSpecByName(wl_spec.network_spec)
        inst.__PrimaryIpv4Allocator = TopoWorkloadConfig.AllocIpv4SubnetAllocator(inst.__NetworkSpec.name)
        inst.__PrimaryIpv4Allocator.Alloc() # To skip 0 ip
        inst.__PrimaryIpv6Allocator = TopoWorkloadConfig.AllocIpv6SubnetAllocator(inst.__NetworkSpec.name)
        inst.__PrimaryIpv6Allocator.Alloc() # To skip 0 ip
        return inst

    def __init__(self, nic_hint, ifname, wlDefn):
        pass

    def GetPrimaryIPv4Allocator(self):
        return self.__PrimaryIpv4Allocator

    def GetPrimaryIPv6Allocator(self):
        return self.__PrimaryIpv6Allocator

    def GetIPv4PrefixLength(self):
        return self.__NetworkSpec.ipv4.prefix_length

    def GetIPv6PrefixLength(self):
        return self.__NetworkSpec.ipv6.prefix_length

    def GetClassicMacAllocator(self):
        return TopoWorkloadConfig.GetClassicMacAllocator()

class NodeWorkloads(object):
    MAX_UDP_PORTS = 8
    MAX_TCP_PORTS = 8



    def __init__(self, wlSpec, default_vlan):
        self.__wlSpec = wlSpec
        self.__node = wlSpec.node
        self.__default_vlan = default_vlan
        if self.__wlSpec.count == 'auto':
            # Uncomment this once the bug PS-724 is fixed
            #num_subifs = (api.Testbed_GetVlanCount() - 1) # 1 for native
            self.__num_subifs = 32 
        else:
            self.__num_subifs = int(self.__wlSpec.count)

        self.__interfaces = {}
        self.__nic_devices = api.GetDeviceNames(self.__node)
        # Since its multi-Naples scenario, regenerate wl.interfaces
        for idx, dev_name in enumerate(self.__nic_devices):
            iflist = []
            hostIfList = api.GetNaplesHostInterfaces(self.GetNodeName(), dev_name)
            for ifnum, hostIntf in enumerate(hostIfList):
                obj = parser.Dict2Object({})
                setattr(obj, 'HostInterface', hostIntf)
                setattr(obj, 'LogicalInterface', 'host_if{0}'.format(ifnum + 1))
                iflist.append(obj)
            self.__interfaces[dev_name] = iflist

        # self api.GetDeviceMapping(node)
        #self.__node_ifs = {}
        #for node in api.GetWorkloadNodeHostnames():
        #    assert(devices)
        #    self.__node_ifs[node] = devices
        self.portUdpAllocator = resmgr.TestbedPortAllocator(20000)
        self.portTcpAllocator = resmgr.TestbedPortAllocator(30000)

        return

    def __add_exposed_ports(self, wl_msg):
        if  wl_msg.workload_type != topo_svc.WORKLOAD_TYPE_CONTAINER:
            return
        for _ in range(NodeWorkloads.MAX_TCP_PORTS):
            tcp_port = wl_msg.exposed_ports.add()
            tcp_port.Port = str(self.portTcpAllocator.Alloc())
            tcp_port.Proto = "tcp"
        for _ in range(NodeWorkloads.MAX_UDP_PORTS):
            udp_port = wl_msg.exposed_ports.add()
            udp_port.Port = str(self.portUdpAllocator.Alloc())
            udp_port.Proto = "udp"      

    def GetNodeName(self):
        return self.__node

    def GetHostInterfaces(self, dev_name):
        pass

    def GetSubInterfaceNetworkSpec(self, nic_hint, subIfIndex, wlDefn):
        vlan = self.__default_vlan
        allocate_vlan = api.GetWorkloadTypeForNode(self.__node) == topo_svc.WorkloadType.Value('WORKLOAD_TYPE_BARE_METAL')
        return SubInterfaceWorkload(nic_hint, subIfIndex, wlDefn, vlan, allocate_vlan)

    def GetNativeNetworkSpec(self, nic_hint, intf, wlDefn):
        return NativeInterfaceWorkload(nic_hint, intf, wlDefn)

    def CreateNativeWorkloads(self, req):
        for device_name, interfaces in self.__interfaces.items(): 
            nic_hint = api.GetDeviceHint(self.__node, device_name)
            for intfObj in interfaces:
                wl_msg = req.workloads.add()
                intf = wl_msg.interfaces.add()
                nw_spec = self.GetNativeNetworkSpec(nic_hint, intfObj.LogicalInterface, self.__wlSpec.spec)
                ipv4_allocator = nw_spec.GetPrimaryIPv4Allocator()
                ipv6_allocator = nw_spec.GetPrimaryIPv6Allocator()
                vlan = 0
                ip4_addr_str = str(ipv4_allocator.Alloc())
                ip6_addr_str = str(ipv6_allocator.Alloc())

                intf.ip_prefix = ip4_addr_str + "/" + str(nw_spec.GetIPv4PrefixLength())
                intf.ipv6_prefix = ip6_addr_str + "/" + str(nw_spec.GetIPv6PrefixLength())
                intf.mac_address = nw_spec.GetClassicMacAllocator().Alloc().get()
                intf.encap_vlan = vlan
                intf.uplink_vlan = intf.encap_vlan
                wl_msg.workload_name = self.GetNodeName() + "_" + intfObj.HostInterface + "_subif_" + str(vlan)
                wl_msg.node_name = self.GetNodeName() # wl.node
                intf.pinned_port = 1
                intf.interface_type = topo_svc.INTERFACE_TYPE_VSS
                intf.interface = intfObj.HostInterface
                intf.parent_interface = intfObj.HostInterface
                wl_msg.workload_type = api.GetWorkloadTypeForNode(self.GetNodeName())
                wl_msg.workload_image = api.GetWorkloadImageForNode(self.GetNodeName())
                wl_msg.mgmt_ip = api.GetMgmtIPAddress(wl_msg.node_name)
                self.__add_exposed_ports(wl_msg)

    def CreateTaggedWorkloads(self, req):

        for device_name, interfaces in self.__interfaces.items(): 
            nic_hint = api.GetDeviceHint(self.__node, device_name)
            for subif_indx in range(self.__num_subifs):
                wl_msg = req.workloads.add()
                intf = wl_msg.interfaces.add()
                intfObj = interfaces[subif_indx % 2] # TODO enhance for unequal distribution of workloads
                nw_spec = self.GetSubInterfaceNetworkSpec(nic_hint, subif_indx, self.__wlSpec.spec)
                ipv4_allocator = nw_spec.GetPrimaryIPv4Allocator() # ipv4_allocators[i]
                ipv6_allocator = nw_spec.GetPrimaryIPv6Allocator() # ipv6_allocators[i]
                vlan = nw_spec.GetVLAN() # vlans[i]
                ip4_addr_str = str(ipv4_allocator.Alloc())
                ip6_addr_str = str(ipv6_allocator.Alloc())
                intf.ip_prefix = ip4_addr_str + "/" + str(nw_spec.GetIPv4PrefixLength())
                intf.ipv6_prefix = ip6_addr_str + "/" + str(nw_spec.GetIPv6PrefixLength())
                intf.mac_address = nw_spec.GetClassicMacAllocator().Alloc().get()
                intf.encap_vlan = vlan
                intf.uplink_vlan = intf.encap_vlan
                wl_msg.node_name = self.GetNodeName()
                intf.pinned_port = 1
                intf.interface_type = topo_svc.INTERFACE_TYPE_VSS
                # node_intf = node_ifs[wl.node][int(intfObj.replace('host_if', '')) - 1]
                if api.GetWorkloadTypeForNode(self.GetNodeName()) == topo_svc.WorkloadType.Value('WORKLOAD_TYPE_BARE_METAL'):
                    intf.interface = intfObj.HostInterface + "_" + str(vlan)
                    wl_msg.workload_name = self.GetNodeName() + "_" + intfObj.HostInterface + "_subif_" + str(vlan)
                else:
                    intf.interface = intfObj.HostInterface + "_mv%d" % (subif_indx+1)
                    wl_msg.workload_name = self.GetNodeName() + "_" + intfObj.HostInterface + "_mv%d" % (subif_indx+1)
                intf.parent_interface = intfObj.HostInterface
                wl_msg.workload_type = api.GetWorkloadTypeForNode(self.GetNodeName())
                wl_msg.workload_image = api.GetWorkloadImageForNode(self.GetNodeName())
                wl_msg.mgmt_ip = api.GetMgmtIPAddress(wl_msg.node_name)
                self.__add_exposed_ports(wl_msg)
                for a in nw_spec.GetSecondaryIpv4Allocator(): # sec_ipv4_allocators[subif_indx]:
                    wl_msg.sec_ip_prefix.append(str(next(a))+"/"+str(nw_spec.ipv4.prefix_length))
                for a in nw_spec.GetSecondaryIpv6Allocator(): # sec_ipv6_allocators[subif_indx]:
                    wl_msg.sec_ipv6_prefix.append(str(next(a))+"/"+str(nw_spec.ipv6.prefix_length))



classic_vlan_map = {}
dvs_vlan_map = {}

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
    ep_spec_ip = ep.spec.ipv6_addresses[0]
    nw_spec_subnet = nw_obj.spec.ipv6_subnet
    ip_str = ep_spec_ip.split('/')[0] + '/' + nw_spec_subnet.split('/')[1]
    return ip_str

def __get_l2segment_vlan_for_endpoint(ep):
    nw_filter = "meta.name=" + ep.spec.network_name + ";"
    objects = netagent_api.QueryConfigs(kind='Network', filter=nw_filter)
    assert(len(objects) == 1)
    return objects[0].spec.vlan_id

def AddConfigWorkloads(req, target_node = None):
    third_party_workload_count = 0
    ep_objs = netagent_api.QueryConfigs(kind='Endpoint')
    ep_ref = None
    for ep in ep_objs:
        node_name = getattr(ep.spec, "_node_name", None)
        if not node_name:
            node_name = ep.spec.node_uuid
        if target_node and target_node != node_name:
            api.Logger.debug("Skipping add workload for node %s" % node_name)
            continue
        if not api.IsNaplesNode(node_name):
            #if api.GetConfigNicMode() == 'hostpin' and third_party_workload_count > 0:
            #    continue
            third_party_workload_count += 1
        req.workload_op = topo_svc.ADD
        wl_msg = req.workloads.add()
        wl_msg.workload_name = ep.meta.name
        wl_msg.node_name = node_name
        intf = wl_msg.interfaces.add()
        intf.ip_prefix = __prepare_ip_address_str_for_endpoint(ep)
        #wl_msg.ipv6_prefix = __prepare_ipv6_address_str_for_endpoint(ep)
        intf.mac_address = ep.spec.mac_address
        intf.pinned_port = 1
        intf.interface_type = topo_svc.INTERFACE_TYPE_VSS

        encap_vlan = getattr(ep.spec, 'useg_vlan', None)
        host_if = None
        if api.GetTestbedNicMode(node_name) == 'hostpin':
            host_if = api.AllocateHostInterfaceForNode(wl_msg.node_name)
            intf.uplink_vlan = __get_l2segment_vlan_for_endpoint(ep)
            if api.GetNicType(wl_msg.node_name) in ['pensando', 'naples']:
                intf.encap_vlan = encap_vlan if encap_vlan else intf.uplink_vlan
            else:
                intf.encap_vlan = intf.uplink_vlan
        elif api.GetTestbedNicMode() == 'hostpin_dvs':
            host_if = api.AllocateHostInterfaceForNode(wl_msg.node_name)
            intf.interface_type = topo_svc.INTERFACE_TYPE_DVS_PVLAN
            intf.switch_name = api.GetVCenterDVSName()
            intf.uplink_vlan = __get_l2segment_vlan_for_endpoint(ep)
            if api.GetNicType(wl_msg.node_name) in ['pensando', 'naples']:
                intf.encap_vlan = ep.spec.primary_vlan
                intf.secondary_encap_vlan = ep.spec.secondary_vlan
            else:
                intf.encap_vlan = intf.uplink_vlan

        elif api.GetTestbedNicMode(node_name) == 'classic':
            global classic_vlan_map
            node_vlan = classic_vlan_map.get(node_name)
            if not node_vlan:
                node_vlan = NodeVlan(node_name)
                classic_vlan_map[node_name] = node_vlan
            #Allocate only if useg_vlan present.
            host_if, wire_vlan = node_vlan.AllocateHostIntfWireVlan(encap_vlan)
            if not node_vlan.IsNativeVlan(wire_vlan):
                #Set encap vlan if its non native.
                intf.encap_vlan = wire_vlan
                intf.uplink_vlan = wire_vlan

        elif api.GetTestbedNicMode(node_name) == 'unified':
            host_if = api.AllocateHostInterfaceForNode(wl_msg.node_name)
            intf.ipv6_prefix = __prepare_ipv6_address_str_for_endpoint(ep)
            intf.uplink_vlan = __get_l2segment_vlan_for_endpoint(ep)
            intf.encap_vlan = intf.uplink_vlan

        else:
            assert(0)

        intf.interface = host_if
        intf.parent_interface = host_if

        wl_msg.workload_type = api.GetWorkloadTypeForNode(wl_msg.node_name)
        wl_msg.workload_image = api.GetWorkloadImageForNode(wl_msg.node_name)
        wl_msg.mgmt_ip = api.GetMgmtIPAddress(wl_msg.node_name)
        wl_msg.cpus = api.GetWorkloadCpusForNode(wl_msg.node_name)
        wl_msg.memory = api.GetWorkloadMemoryForNode(wl_msg.node_name)

def AddConfigClassicWorkloads(req, target_node = None):
    # Using up first vlan for native

    req.workload_op = topo_svc.ADD
    api.Logger.info("Allocating native VLAN") 
    native_vlan = api.Testbed_AllocateVlan()
    # Forming native workloads
    for workload in TopoWorkloadConfig.GetNativeWorkloadInstances():
        wl = workload.workload
        node_wlm = NodeWorkloads(wl, native_vlan)
        node_wlm.CreateNativeWorkloads(req)

    api.Logger.info("Allocating tagged VLAN") 
    tagged_vlan = api.Testbed_AllocateVlan()
    for workload in TopoWorkloadConfig.GetTaggedWorkloadInstances():
        wl = workload.workload
        node_wlm = NodeWorkloads(wl, tagged_vlan)
        node_wlm.CreateTaggedWorkloads(req)

    return


