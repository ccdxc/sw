#! /usr/bin/python3
import sys
import pdb
import os
import ipaddress
import time
import iota.test.iris.config.netagent.api as netagent_api


import iota.harness.api as api
import iota.test.iris.config.netagent.api as agent_api

import iota.protos.pygen.topo_svc_pb2 as topo_svc


def __prepare_ip_address_str_for_endpoint(ep):
    nw_filter = "meta.name=" + ep.spec.network_name + ";"
    objects = netagent_api.QueryConfigs(kind='Network', filter=nw_filter)
    assert(len(objects) == 1)
    nw_obj = objects[0]
    ep_spec_ip = ep.spec.ipv4_addresses[0]
    nw_spec_subnet = nw_obj.spec.ipv4_subnet
    ip_str = ep_spec_ip.split('/')[0] + '/' + nw_spec_subnet.split('/')[1]
    return ip_str


def __get_l2segment_vlan_for_endpoint(ep):
    objects = netagent_api.QueryConfigs(kind='Network', attribute="meta.name", value=ep.spec.network_name)
    assert(len(objects) == 1)
    return objects[0].spec.vlan_id

def __init_lifdb():
    global gl_lifdb
    gl_lifdb = {}
    node_names = api.GetNaplesHostnames()
    for name in node_names:
        gl_lifdb[name] = []
        for idx in range(16):
            lif_id = "lif%d" % (idx + 100)
            uplink = (idx % 2) + 1
            gl_lifdb[name].append((lif_id, uplink))


def __alloc_lif(node_name):
    global gl_lifdb
    lif_uplink = gl_lifdb[node_name][0]
    del gl_lifdb[node_name][0]
    return lif_uplink


def __add_workloads():
    ep_objs = netagent_api.QueryConfigs(kind='Endpoint')
    for ep in ep_objs.endpoints:
        req = topo_svc.WorkloadMsg()
        req.workload_op = topo_svc.ADD
        wl_msg = req.workloads.add()
        wl_msg.workload_name = ep.meta.name
        wl_msg.node_name = getattr(ep.spec, "node-uuid", None)
        wl_msg.encap_vlan = getattr(ep.spec, 'useg-vlan')
        wl_msg.ip_prefix = __prepare_ip_address_str_for_endpoint(ep)
        wl_msg.mac_address = getattr(ep.spec, 'mac-address')
        lif_uplink = __alloc_lif(wl_msg.node_name)
        wl_msg.interface = lif_uplink[0]
        wl_msg.parent_interface = lif_uplink[0]
        wl_msg.pinned_port = lif_uplink[1]
        wl_msg.interface_type = topo_svc.INTERFACE_TYPE_SRIOV
        wl_msg.uplink_vlan = __get_l2segment_vlan_for_endpoint(ep)
        wl_msg.workload_type = api.GetWorkloadTypeForNode(wl_msg.node_name)
        wl_msg.workload_image = api.GetWorkloadImageForNode(wl_msg.node_name)
        resp = api.AddWorkloads(req)


def Main(step):
    # time.sleep(120)
    #api.Init()
    agent_ips = api.GetNaplesMgmtIpAddresses()
    netagent_api.Init(agent_ips)

    netagent_api.ReadConfigs(api.GetTopologyDirectory())
    __init_lifdb()

    netagent_api.DeleteBaseConfig()
    if api.GetNicMode() != 'classic':
        netagent_api.PushBaseConfig()

    if not api.IsConfigOnly():
        __add_workloads()
    return api.types.status.SUCCESS


if __name__ == '__main__':
    Main(None)
