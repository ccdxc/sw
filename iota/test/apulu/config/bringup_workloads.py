#! /usr/bin/python3
import pdb
import os
import json
import sys

import iota.harness.api as api
import iota.harness.infra.utils.parser as parser
import iota.test.apulu.config.api as config_api

import iota.protos.pygen.topo_svc_pb2 as topo_svc



def __add_workloads():

    req = topo_svc.WorkloadMsg()

    req.workload_op = topo_svc.ADD
    for ep in config_api.GetEndpoints():
        wl_msg = req.workloads.add()
        # Make the workload_name unique across nodes by appending node-name
        wl_msg.workload_name = ep.name + ep.node_name
        wl_msg.node_name = ep.node_name
        wl_msg.ip_prefix = ep.ip_addresses[0]
        # wl_msg.ipv6_prefix = ep.ip_addresses[1]
        wl_msg.mac_address = str(ep.macaddr)
        #wl_msg.interface_type = topo_svc.INTERFACE_TYPE_VSS
        wl_msg.interface_type = topo_svc.INTERFACE_TYPE_NONE
        wl_msg.encap_vlan = ep.vlan
        wl_msg.interface = api.AllocateHostInterfaceForNode(wl_msg.node_name)
        wl_msg.parent_interface = wl_msg.interface
        wl_msg.workload_type = api.GetWorkloadTypeForNode(wl_msg.node_name)
        wl_msg.workload_image = api.GetWorkloadImageForNode(wl_msg.node_name)
        api.Logger.info("Workload-name %s node-name %s, ip-prefix %s mac %s" % (wl_msg.workload_name, wl_msg.node_name, wl_msg.ip_prefix, str(ep.macaddr)))

    if len(req.workloads):
        api.Logger.info("Adding %d Workloads" % len(req.workloads))
        resp = api.AddWorkloads(req, skip_bringup=api.IsConfigOnly())
        if resp is None:
            sys.exit(1)



def Main(step):
    api.Logger.info("Adding Workloads")
    __add_workloads()
    return api.types.status.SUCCESS

if __name__ == '__main__':
    Main(None)
