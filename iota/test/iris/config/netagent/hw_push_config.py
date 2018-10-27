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

import iota.protos.pygen.types_pb2 as types_pb2
import iota.protos.pygen.topo_svc_pb2 as topo_svc

gl_ep_json_obj = None
gl_nw_json_obj = None
gl_sg_json_obj = None
gl_host_if_idx = 0

def __read_one_json(filename):
    json_file_path = "%s/%s" % (api.GetTopologyDirectory(), filename)
    api.Logger.info("Reading config JSON file: %s" % json_file_path)
    return api.parser.JsonParse(json_file_path)

def __read_jsons():
    global gl_ep_json_obj
    gl_ep_json_obj = __read_one_json('endpoints.json')
    agent_uuid_map = api.GetNaplesNodeUuidMap()
    for obj in gl_ep_json_obj.endpoints:
        node_name = getattr(obj.spec, "node-uuid", None)
        assert(node_name)
        setattr(obj.spec, "node-uuid", "%s" % agent_uuid_map[node_name])
    
    global gl_nw_json_obj
    gl_nw_json_obj = __read_one_json('networks.json')

    global gl_sg_json_obj
    gl_sg_json_obj = __read_one_json('sgpolicy.json')
    return

def __config():
    agent_api.ConfigureNetworks(gl_nw_json_obj.networks)
    agent_api.ConfigureEndpoints(gl_ep_json_obj.endpoints)
    agent_api.ConfigureSecurityGroupPolicies(gl_sg_json_obj.sgpolices)
    return api.types.status.SUCCESS

def __find_network(nw_name):
    for nw in gl_nw_json_obj.networks:
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

def __init_host_if_db():
    global gl_host_if_db
    gl_host_if_db = {}
    node_names = api.GetWorkloadNodeHostnames()
    for name in node_names:
        gl_host_if_db[name] = api.GetWorkloadNodeHostInterfaces(name)
    return

def __alloc_host_if(node_name):
    global gl_host_if_db
    global gl_host_if_idx
    num_host_ifs = len(gl_host_if_db[node_name])
    host_if = gl_host_if_db[node_name][gl_host_if_idx]
    gl_host_if_idx = (gl_host_if_idx + 1) % num_host_ifs
    return host_if

def __add_workloads():
    ep_objs = __read_one_json('endpoints.json')
    for ep in ep_objs.endpoints:
        req = topo_svc.WorkloadMsg()
        req.workload_op = topo_svc.ADD
        wl_msg = req.workloads.add()
        wl_msg.workload_name = ep.meta.name 
        wl_msg.node_name = getattr(ep.spec, "node-uuid", None)
        wl_msg.ip_prefix = __prepare_ip_address_str_for_endpoint(ep)
        wl_msg.mac_address = getattr(ep.spec, 'mac-address')
        host_if = __alloc_host_if(wl_msg.node_name)
        wl_msg.interface = host_if
        wl_msg.pinned_port = 1
        wl_msg.interface_type = topo_svc.INTERFACE_TYPE_VSS
        wl_msg.uplink_vlan = __get_l2segment_vlan_for_endpoint(ep)
        encap_vlan = getattr(ep.spec, 'useg-vlan', None)
        wl_msg.encap_vlan = encap_vlan if encap_vlan else wl_msg.uplink_vlan
        wl_msg.workload_type = api.GetWorkloadTypeForNode(wl_msg.node_name)
        wl_msg.workload_image = api.GetWorkloadImageForNode(wl_msg.node_name)

        resp = api.AddWorkloads(req)

def Main(step):
    #time.sleep(120)
    api.Logger.SetLoggingLevel(api.types.loglevel.DEBUG)
    api.Init()
    agent_ips = api.GetNaplesMgmtIpAddresses()
    agent_api.Init(agent_ips, hw = True)

    __read_jsons()
    __init_host_if_db() 
    if api.GetNicMode() != 'classic':
        __config()

    __add_workloads()
    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
