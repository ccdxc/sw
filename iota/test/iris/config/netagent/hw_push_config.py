#! /usr/bin/python3
import sys
import pdb
import os
import ipaddress
import time

import iota.harness.api as api
import iota.test.iris.config.netagent.api as netagent_api

import iota.protos.pygen.types_pb2 as types_pb2
import iota.protos.pygen.topo_svc_pb2 as topo_svc


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

def __add_workloads():
    third_party_workload_count = 0

    ep_objs = netagent_api.QueryConfigs(kind='Endpoint')
    req = topo_svc.WorkloadMsg()
    for ep in ep_objs:

        node_name = getattr(ep.spec, "_node_name", None)
        if not node_name:
            node_name = ep.spec.node_uuid
        if not api.IsNaplesNode(node_name):
            if third_party_workload_count > 0:
                continue
            else:
                third_party_workload_count += 1
        req.workload_op = topo_svc.ADD
        wl_msg = req.workloads.add()
        wl_msg.workload_name = ep.meta.name
        wl_msg.node_name = node_name
        wl_msg.ip_prefix = __prepare_ip_address_str_for_endpoint(ep)
        wl_msg.ipv6_prefix = __prepare_ipv6_address_str_for_endpoint(ep)
        wl_msg.mac_address = ep.spec.mac_address
        host_if = api.AllocateHostInterfaceForNode(wl_msg.node_name)
        wl_msg.interface = host_if
        wl_msg.pinned_port = 1
        wl_msg.interface_type = topo_svc.INTERFACE_TYPE_VSS
        if api.GetNicMode() != 'classic':
            wl_msg.uplink_vlan = __get_l2segment_vlan_for_endpoint(ep)
            if api.GetNicType(wl_msg.node_name) == 'pensando':
                encap_vlan = getattr(ep.spec, 'useg_vlan', None)
                wl_msg.encap_vlan = encap_vlan if encap_vlan else wl_msg.uplink_vlan
            else:
                wl_msg.encap_vlan = wl_msg.uplink_vlan

        wl_msg.workload_type = api.GetWorkloadTypeForNode(wl_msg.node_name)
        wl_msg.workload_image = api.GetWorkloadImageForNode(wl_msg.node_name)

    resp = api.AddWorkloads(req)
    if resp is None:
        sys.exit(1)

#This function is hack for now as we need add workloads again if driver reloaded
def AddWorkloads():
    __add_workloads()



def Main(step):
    #time.sleep(120)
    agent_ips = api.GetNaplesMgmtIpAddresses()
    netagent_api.Init(agent_ips, hw = True)

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
