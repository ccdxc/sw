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
import iota.test.iris.config.workload.base as wl_orch
import iota.harness.infra.resmgr as resmgr
from iota.harness.infra.glopts import GlobalOptions as GlobalOptions

import iota.protos.pygen.topo_svc_pb2 as topo_svc


vmotion_mac_allocator = resmgr.MacAddressStep("00CC.0000.0001", "0000.0000.0001")
vmotion_ip_allocator = resmgr.IpAddressStep("172.16.0.1", "0.0.0.1")


def __get_l2segment_vlan_for_endpoint(ep):
    nw_filter = "meta.name=" + ep.spec.network_name + ";"
    objects = netagent_api.QueryConfigs(kind='Network', filter=nw_filter)
    assert(len(objects) == 1)
    return objects[0].spec.vlan_id

def __setup_vmotion_on_hosts():
    vmotion_workloads = {}
    ep_objs = netagent_api.QueryConfigs(kind='Endpoint')
    ep_ref = None
    for ep in ep_objs:
        node_name = getattr(ep.spec, "_node_name", None)
        if not node_name:
            node_name = ep.spec.node_uuid    
        l2seg = __get_l2segment_vlan_for_endpoint(ep)
        if  api.GetTestbedNicMode(node_name) == 'hostpin_dvs' and not vmotion_workloads.get(node_name, None) and l2seg != 0:
            ep_ref = copy.deepcopy(ep)
            ep_ref.spec.mac_address = vmotion_mac_allocator.Alloc().get()
            ep_ref.spec.ipv4_addresses = [str(vmotion_ip_allocator.Alloc())]
            ep_ref.meta.name = "vmotion-" + str(node_name)
            ret = netagent_api.PushConfigObjects([ep_ref], ignore_error=False)
            if ret != api.types.status.SUCCESS:
                api.Logger.info("Failed to push vmotion endpoint")
            vmotion_workloads[node_name] = ep_ref   

    workloads = api.GetWorkloads()
    hostMap = {}
    for workload in workloads:
        if not hostMap.get(workload.node_name, None):
            if vmotion_workloads[workload.node_name].spec.useg_vlan == workload.encap_vlan:
                vmotion_workloads[workload.node_name].network_name = workload.network_name

    for host, workload in vmotion_workloads.items():
        ret = api.EnableVmotionOnNetwork(host, workload.network_name, workload.spec.mac_address)
        if ret != api.types.status.SUCCESS:
            return ret
    return api.types.status.SUCCESS


def __add_workloads(target_node = None):
    req = topo_svc.WorkloadMsg()
    if api.GetTestbedNicMode(target_node) in ['hostpin', 'hostpin_dvs', 'unified']:
        wl_orch.AddConfigWorkloads(req, target_node)
    elif api.GetTestbedNicMode(target_node) == 'classic':
        wl_orch.AddConfigClassicWorkloads(req, target_node)
    else:
        assert(0)

    if len(req.workloads):
        resp = api.AddWorkloads(req, skip_bringup=api.IsConfigOnly())
        if resp is None:
            sys.exit(1)
    if api.GetTestbedNicMode(target_node) == 'hostpin_dvs':
        ret = __setup_vmotion_on_hosts()
        if ret != api.types.status.SUCCESS:
            sys.exit(1)

def __recover_workloads(target_node = None):
    objects = netagent_api.QueryConfigs(kind='Endpoint')
    netagent_api.UpdateNodeUuidEndpoints(objects)
    req = topo_svc.WorkloadMsg()
    resp = api.RestoreWorkloads(req)
    if resp is None:
        sys.exit(1)

def GetIPv4Allocator(nw_name):
    ipv4_subnet_allocator = wl_orch.TopoWorkloadConfig.AllocIpv4SubnetAllocator(nw_name)
    if ipv4_subnet_allocator:
        subnet = copy.deepcopy(ipv4_subnet_allocator)
        return subnet
    return None

def GetIPv6Allocator(nw_name):
    ipv6_subnet_allocator = wl_orch.TopoWorkloadConfig.AllocIpv6SubnetAllocator(nw_name)
    if ipv6_subnet_allocator:
        subnet = copy.deepcopy(ipv6_subnet_allocator)
        return subnet
    return None

def GetMacAllocator(): 
    return copy.deepcopy(wl_orch.TopoWorkloadConfig.GetClassicMacAllocator())

def AddWorkloads(target_node = None):
    __add_workloads(target_node)

def RestoreWorkloads():
    __recover_workloads()

def DeleteWorkloads(target_node=None):
    if api.GetTestbedNicMode(target_node) == 'classic':
        __delete_classic_workloads()
    else:
        __delete_workloads()

def AddNaplesWorkloads(target_node=None):
    req = topo_svc.WorkloadMsg()
    req.workload_op = topo_svc.ADD
    wloads = api.GetNaplesWorkloads()
    for wl in wloads:
        if wl.SkipNodePush() or (target_node and target_node != wl.node_name):
            api.Logger.debug("Skipping add workload for node %s" % wl.node_name)
            continue
        wl_msg = req.workloads.add()
        intf = wl_msg.interfaces.add()
        wl_msg.workload_name = wl.workload_name
        wl_msg.node_name = wl.node_name
        intf.ip_prefix = wl.ip_address + "/" + '24'
        #wl_msg.ipv6_prefix = __prepare_ipv6_address_str_for_endpoint(ep)
        intf.interface = wl.interface
        intf.parent_interface = wl.interface
        intf.interface_type = topo_svc.INTERFACE_TYPE_VSS
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
    for wl in api.GetWorkloads():
        if target_node and target_node != wl.node_name:
            api.Logger.debug("Skipping add classic workload for node %s" % wl.node_name)
            continue
        wl_msg = req.workloads.add()
        intf = wl_msg.interfaces.add()
        intf.ip_prefix = wl.ip_prefix
        intf.ipv6_prefix = wl.ipv6_prefix
        intf.mac_address = wl.mac_address
        intf.encap_vlan = wl.encap_vlan
        intf.uplink_vlan = wl.uplink_vlan
        wl_msg.workload_name = wl.workload_name
        wl_msg.node_name = wl.node_name
        intf.pinned_port = wl.pinned_port
        intf.interface_type = wl.interface_type
        intf.interface = wl.parent_interface
        intf.parent_interface = wl.parent_interface
        wl_msg.workload_type = wl.workload_type
        wl_msg.workload_image = wl.workload_image
        wl_msg.mgmt_ip = api.GetMgmtIPAddress(wl_msg.node_name)
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
    if api.GetTestbedNicMode(node) == 'classic':
        __delete_classic_workloads(node)
        __readd_classic_workloads(node)
    else:
        __delete_workloads(node)
        __add_workloads(node)
    AddNaplesWorkloads(node)

def UpdateNetworkAndEnpointObject():
    nwObj = netagent_api.QueryConfigs(kind='Network')
    if not nwObj:
        api.Logger.error("Failed to get network object")
        return api.types.status.FAILURE

    api.Testbed_ResetVlanAlloc()
    vlan = api.Testbed_AllocateVlan()
    api.Logger.info("Ignoring first vlan as it is native ", vlan)
    netagent_api.UpdateTestBedVlans(nwObj)

    epObj = netagent_api.QueryConfigs(kind='Endpoint')
    if not epObj:
        api.Logger.error("Failed to get endpoint object")
        return api.types.status.FAILURE

    netagent_api.UpdateNodeUuidEndpoints(epObj)
    return api.types.status.SUCCESS

def Main(args):
    #time.sleep(120)
    api.Logger.info("Testsuite NIC Mode is %s"%(api.GetConfigNicMode()))
    agent_nodes = api.GetNaplesHostnames()
    netagent_api.Init(agent_nodes, hw = True)

    netagent_api.ReadConfigs(api.GetTopologyDirectory(), reset=False)

    if api.GetConfigNicMode() in ['unified']:
        ret = UpdateNetworkAndEnpointObject()
        if ret != api.types.status.SUCCESS:
            return ret
    
    #Delete path is not stable yet
    #netagent_api.DeleteBaseConfig()

    if GlobalOptions.skip_setup:
        RestoreWorkloads()
    else:
        if api.GetConfigNicMode() not in ['classic','unified']:
            netagent_api.PushBaseConfig()
        __add_workloads()
    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
