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
import iota.test.iris.netagent.config.api as agent_api

import iota.protos.pygen.types_pb2 as types_pb2
import iota.protos.pygen.topo_svc_pb2 as topo_svc

gl_ep_json_obj = None
gl_nw_json_obj = None
gl_sg_json_obj = None

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
        setattr(obj.spec, "node-uuid", "uuid-%s" % agent_uuid_map[node_name])
    
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
    ep_objs = __read_one_json('endpoints.json')
    for ep in ep_objs.endpoints:
        req = topo_svc.WorkloadMsg()
        req.workload_op = topo_svc.ADD
        ep_msg = req.workloads.add()
        ep_msg.workload_name = ep.meta.name 
        ep_msg.node_name = getattr(ep.spec, "node-uuid", None)
        ep_msg.encap_vlan = getattr(ep.spec, 'useg-vlan')
        ep_msg.ip_prefix = __prepare_ip_address_str_for_endpoint(ep)
        ep_msg.mac_address = getattr(ep.spec, 'mac-address')
        lif_uplink = __alloc_lif(ep_msg.node_name)
        ep_msg.interface = lif_uplink[0]
        ep_msg.pinned_port = lif_uplink[1]
        ep_msg.interface_type = topo_svc.INTERFACE_TYPE_SRIOV
        ep_msg.uplink_vlan = __get_l2segment_vlan_for_endpoint(ep)
        resp = api.AddWorkloads(req)

def Main(step):
    #time.sleep(120)
    api.Logger.SetLoggingLevel(api.types.loglevel.DEBUG)
    api.Init()
    agent_ips = api.GetNaplesMgmtIpAddresses()
    agent_api.Init(agent_ips)

    __read_jsons()
    __init_lifdb() 
    __config()
    __add_workloads()
    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
