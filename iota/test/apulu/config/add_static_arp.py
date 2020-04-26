#! /usr/bin/python3
import pdb
import os
import json
import sys

import iota.harness.api as api
import iota.harness.infra.utils.parser as parser
import iota.test.apulu.config.api as config_api

import iota.protos.pygen.topo_svc_pb2 as topo_svc


def __get_arp_add_cmd(ip_addr, mac_addr, intf, args=""):
    arp_base_cmd = "ip %s neigh add %s lladdr %s nud permanent dev %s"
    arp_cmd = arp_base_cmd % (args, ip_addr, mac_addr, intf)
    return arp_cmd

def __add_static_arp():

    workload_pairs = config_api.GetPingableWorkloadPairs()

    if len(workload_pairs) == 0:
        api.Logger.info("No workload pairs to setup static arp")
        return api.types.status.FAILURE

    #req = api.Trigger_CreateAllParallelCommandsRequest()
    req = api.Trigger_CreateExecuteCommandsRequest()
    for pair in workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        arp_cmd = __get_arp_add_cmd(w1.ip_address, w1.mac_address, w2.interface)
        api.Trigger_AddCommand(req, w2.node_name, w2.workload_name, arp_cmd)
        #arp_cmd = __get_arp_add_cmd(w1.ipv6_address, w1.mac_address, w2.interface, "-6")
        #api.Trigger_AddCommand(req, w2.node_name, w2.workload_name, arp_cmd)

    resp = api.Trigger(req)


    for cmd in resp.commands:
        if cmd.exit_code != 0:
            api.Logger.error("Failed to install static arp entries")
            api.PrintCommandResults(cmd)
            return api.types.status.FAILURE

    return api.types.status.SUCCESS



def Main(step):
    api.Logger.info("Adding Static ARP entries")
    return __add_static_arp()

if __name__ == '__main__':
    Main(None)
