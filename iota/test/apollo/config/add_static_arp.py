#! /usr/bin/python3
import pdb
import os
import json
import sys

import iota.harness.api as api
import iota.harness.infra.utils.parser as parser
import iota.test.apollo.config.api as config_api

import iota.protos.pygen.topo_svc_pb2 as topo_svc



def __add_static_arp():

    workload_pairs = config_api.GetPingableWorkloadPairs()

    if len(workload_pairs) == 0:
        api.Logger.info("No workload pairs to setup static arp")
        return api.types.status.FAILURE

    req = api.Trigger_CreateAllParallelCommandsRequest()
    for pair in workload_pairs:
        w1 = pair[0]
        w2 = pair[1]
        arp_cmd = "arp -s " + w1.ip_address + " " + w1.mac_address
        api.Trigger_AddCommand(req, w2.node_name, w2.workload_name,
                               arp_cmd)
        arp_cmd = "arp -s " + w2.ip_address + " " + w2.mac_address
        api.Trigger_AddCommand(req, w1.node_name, w1.workload_name,
                               arp_cmd)


    resp = api.Trigger(req)


    for cmd in resp.commands:
        if cmd.exit_code != 0:
            return api.types.status.FAILURE

    return api.types.status.SUCCESS



def Main(step):
    api.Logger.info("Adding Static ARP entries")
    return __add_static_arp()

if __name__ == '__main__':
    Main(None)
