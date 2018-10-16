#! /usr/bin/python3
import sys
import pdb
import os
import ipaddress
import time

if __name__ == '__main__':
    topdir = os.path.dirname(sys.argv[0]) + '../../../../../'
    topdir = os.path.abspath(topdir)
    print(topdir)
    sys.path.insert(0, topdir)
    fullpath = os.path.join(topdir, 'iota/protos/pygen')
    sys.path.append(fullpath)

import iota.harness.api as api
import iota.test.iris.netagent.restclient.api as agent_api

import iota.protos.pygen.types_pb2 as types_pb2
import iota.protos.pygen.topo_svc_pb2 as topo_svc

CONFIGS_DIR = "test/iris/netagent/bringup/configs/1ten_2seg_4eps"

ENDPOINTS_JSON = "%s/endpoints.json" % CONFIGS_DIR
NETWORKS_JSON = "%s/networks.json" % CONFIGS_DIR
SGPOLICY_JSON = "%s/sgpolicy.json" % CONFIGS_DIR

ep_json_obj = None
nw_json_obj = None
sg_json_obj = None

def __read_jsons():
    global ep_json_obj
    ep_json_obj = api.parser.JsonParse(ENDPOINTS_JSON)

    global nw_json_obj
    nw_json_obj = api.parser.JsonParse(NETWORKS_JSON)

    global sg_json_obj
    sg_json_obj = api.parser.JsonParse(SGPOLICY_JSON)
    return

def __config():
    agent_api.ConfigureNetworks(nw_json_obj.networks)
    agent_api.ConfigureEndpoints(ep_json_obj.endpoints)
    agent_api.ConfigureSecurityGroupPolicies(sg_json_obj.sgpolices)
    return api.types.status.SUCCESS

def __find_network(nw_name):
    for nw in nw_json_obj.networks:
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
    for ep in ep_json_obj.endpoints:
        req = topo_svc.WorkloadMsg()
        req.workload_op = topo_svc.ADD
        ep_msg = req.workloads.add()
        ep_msg.workload_name = ep.meta.name 
        ep_msg.node_name = 'naples1'
        ep_msg.encap_vlan = getattr(ep.spec, 'useg-vlan')
        ep_msg.ip_address = __prepare_ip_address_str_for_endpoint(ep)
        ep_msg.mac_address = getattr(ep.spec, 'mac-address')
        ep_msg.interface = ep.spec.interface
        ep_msg.interface_type = topo_svc.INTERFACE_TYPE_SRIOV
        ep_msg.uplink_vlan = __get_l2segment_vlan_for_endpoint(ep)
        resp = api.AddWorkloads(req)

def Main(step):
    #time.sleep(120)
    api.Logger.SetLoggingLevel(api.types.loglevel.DEBUG)
    api.Init()
    agent_ips = api.GetNaplesMgmtIpAddresses()
    agent_uuids = api.GetNaplesNodeUUIDs()
    agent_api.Init(agent_ips[0], agent_uuids[0])

    __read_jsons()
    __config()
    __add_workloads()
    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
