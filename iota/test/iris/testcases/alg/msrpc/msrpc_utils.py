#! /usr/bin/python3
import os
import iota.harness.api as api

MSRPC_PORT = 135

dir_path = os.path.dirname(os.path.realpath(__file__))

def GetTcpdumpData(node):
    resp = api.CopyFromWorkload(node.node_name, node.workload_name, ['out.txt'], dir_path)
    if resp is None:
       return None
