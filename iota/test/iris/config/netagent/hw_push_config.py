#! /usr/bin/python3
import sys
import pdb
import os
import ipaddress
import time
from collections import defaultdict

import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api

import iota.protos.pygen.types_pb2 as types_pb2
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


classic_vlan_map = {}

def __prepare_ip_address_str_for_endpoint(ep):
    nw_filter = "meta.name=" + ep.spec.network_name + ";"
    objects = netagent_api.QueryConfigs(kind='Network', filter=nw_filter)
    assert(len(objects) == 1)
    nw_obj = objects[0]
    ep_spec_ip = ep.spec.ipv4_address
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
            api.Logger.info("Skipping add workload for node %s" % node_name)
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
        wl_msg.ipv6_prefix = __prepare_ipv6_address_str_for_endpoint(ep)
        wl_msg.mac_address = ep.spec.mac_address
        wl_msg.pinned_port = 1
        wl_msg.interface_type = topo_svc.INTERFACE_TYPE_VSS
        encap_vlan = getattr(ep.spec, 'useg_vlan', None)
        host_if = None
        if api.GetNicMode() == 'hostpin':
            host_if = api.AllocateHostInterfaceForNode(wl_msg.node_name)
            wl_msg.uplink_vlan = __get_l2segment_vlan_for_endpoint(ep)
            if api.GetNicType(wl_msg.node_name) == 'pensando':
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

        wl_msg.workload_type = api.GetWorkloadTypeForNode(wl_msg.node_name)
        wl_msg.workload_image = api.GetWorkloadImageForNode(wl_msg.node_name)

def __add_workloads(target_node = None):
    req = topo_svc.WorkloadMsg()
    __add_config_worklads(req, target_node)
    if len(req.workloads):
        resp = api.AddWorkloads(req)
        if resp is None:
            sys.exit(1)

#This function is hack for now as we need add workloads again if driver reloaded
def AddWorkloads():
    __add_workloads()

def AddNaplesWorkloads(target_node=None):
    req = topo_svc.WorkloadMsg()
    req.workload_op = topo_svc.ADD
    wloads = api.GetNaplesWorkloads()
    for wl in wloads:
        if wl.SkipNodePush() or (target_node and target_node != wl.node_name):
            api.Logger.info("Skipping add workload for node %s" % wl.node_name)
            continue
        wl_msg = req.workloads.add()
        wl_msg.workload_name = wl.workload_name
        wl_msg.node_name = wl.node_name
        wl_msg.ip_prefix = wl.ip_address + "/" + '24'
        #wl_msg.ipv6_prefix = __prepare_ipv6_address_str_for_endpoint(ep)
        wl_msg.interface = wl.interface
        wl_msg.interface_type = topo_svc.INTERFACE_TYPE_VSS
        wl_msg.workload_type  = topo_svc.WorkloadType.Value('WORKLOAD_TYPE_BARE_METAL')
    if len(req.workloads):
        resp = api.AddWorkloads(req, skip_store=True)
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
    __delete_workloads(node)
    __add_workloads(node)
    AddNaplesWorkloads(node)



def Main(step):
    #time.sleep(120)
    agent_nodes = api.GetNaplesHostnames()
    netagent_api.Init(agent_nodes, hw = True)

    netagent_api.ReadConfigs(api.GetTopologyDirectory())
    #Delete path is not stable yet
    #netagent_api.DeleteBaseConfig()
    if api.GetNicMode() != 'classic':
        netagent_api.PushBaseConfig()

    if not api.IsConfigOnly():
        __add_workloads()
    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
