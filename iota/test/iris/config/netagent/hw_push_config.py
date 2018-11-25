#! /usr/bin/python3
import sys
import pdb
import os
import ipaddress
import time
'''
if __name__ == '__main__':
    topdir = os.path.dirname(sys.argv[0]) + '../../../../../'
    topdir = os.path.abspath(topdir)
    print(topdir)
    sys.path.insert(0, topdir)
    fullpath = os.path.join(topdir, 'iota/protos/pygen')
    sys.path.append(fullpath)
'''

import iota.harness.api as api
import iota.test.iris.config.netagent.api as agent_api
import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api

import iota.protos.pygen.types_pb2 as types_pb2
import iota.protos.pygen.topo_svc_pb2 as topo_svc

def __find_network(nw_name):
    for nw in netagent_cfg_api.gl_nw_json_obj.networks:
        if nw.meta.name == nw_name:
            return nw
    return None

def __prepare_ip_address_str_for_endpoint(ep):
    nw_name = getattr(ep.spec, 'network-name')
    nw_obj = __find_network(nw_name)
    assert(nw_obj)
    ep_spec_ip = getattr(ep.spec, 'ipv4-address')
    nw_spec_subnet = getattr(nw_obj.spec, 'ipv4-subnet')
    ip_str = str(ipaddress.ip_network(ep_spec_ip).network_address) + '/' + \
             str(ipaddress.ip_network(nw_spec_subnet).prefixlen)
    return ip_str

def __get_l2segment_vlan_for_endpoint(ep):
    nw_name = getattr(ep.spec, 'network-name')
    nw_obj = __find_network(nw_name)
    assert(nw_obj)
    return getattr(nw_obj.spec, 'vlan-id')

def __add_workloads():
    ep_objs = netagent_cfg_api.gl_ep_json_obj
    for ep in ep_objs.endpoints:
        req = topo_svc.WorkloadMsg()
        req.workload_op = topo_svc.ADD
        wl_msg = req.workloads.add()
        wl_msg.workload_name = ep.meta.name
        wl_msg.node_name = getattr(ep.spec, "node-uuid", None)
        wl_msg.ip_prefix = __prepare_ip_address_str_for_endpoint(ep)
        wl_msg.mac_address = getattr(ep.spec, 'mac-address')
        host_if = api.AllocateHostInterfaceForNode(wl_msg.node_name)
        wl_msg.interface = host_if
        wl_msg.pinned_port = 1
        wl_msg.interface_type = topo_svc.INTERFACE_TYPE_VSS
        if api.GetNicMode() != 'classic':
            wl_msg.uplink_vlan = __get_l2segment_vlan_for_endpoint(ep)
            if api.GetNicType(wl_msg.node_name) == 'pensando':
                encap_vlan = getattr(ep.spec, 'useg-vlan', None)
                wl_msg.encap_vlan = encap_vlan if encap_vlan else wl_msg.uplink_vlan
            else:
                wl_msg.encap_vlan = wl_msg.uplink_vlan

        wl_msg.workload_type = api.GetWorkloadTypeForNode(wl_msg.node_name)
        wl_msg.workload_image = api.GetWorkloadImageForNode(wl_msg.node_name)

        resp = api.AddWorkloads(req)

#This function is hack for now as we need add workloads again if driver reloaded
def AddWorkloads():
    __add_workloads()


def Main(step):
    #time.sleep(120)
    agent_ips = api.GetNaplesMgmtIpAddresses()
    agent_api.Init(agent_ips, hw = True)

    netagent_cfg_api.ReadJsons()
    if api.GetNicMode() != 'classic':
        netagent_cfg_api.PushConfig()

    if not api.IsConfigOnly():
        __add_workloads()
    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
