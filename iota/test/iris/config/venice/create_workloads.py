#! /usr/bin/python3
import json
import pdb

import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc
import iota.protos.pygen.types_pb2 as types_pb2

def Main(tc):
    generated_configs = api.GetVeniceConfigs()

    ip_index = 101
    for cfg_object in generated_configs:
        obj = json.loads(cfg_object.Config)        
        if obj["kind"] != "Workload": continue
        req = topo_svc.WorkloadMsg()
        req.workload_op = topo_svc.ADD
        wl = req.workloads.add()
        wl.workload_name = obj["meta"]["name"]
        
        wl.node_name = obj["spec"]["host-name"]
        if_obj = obj["spec"]["interfaces"][0]
        wl.mac_address = if_obj["mac-address"]
        wl.encap_vlan = if_obj["micro-seg-vlan"]
        wl.uplink_vlan = if_obj["external-vlan"]
        wl.ip_prefix = "192.168.%d.%d/24" % (wl.uplink_vlan, ip_index)
        wl.interface = 'lif100'
        wl.pinned_port = 1
        wl.interface_type = topo_svc.INTERFACE_TYPE_VSS
        wl.workload_type = api.GetWorkloadTypeForNode(wl.node_name)
        wl.workload_image = api.GetWorkloadImageForNode(wl.node_name)
        ip_index += 1

        resp = api.AddWorkloads(req)
    return api.types.status.SUCCESS
