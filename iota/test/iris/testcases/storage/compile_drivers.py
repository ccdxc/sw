#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import os

def Setup(tc):
    topdir = api.GetTopDir()
    fullpath = topdir + '/' + tc.args.drivers
    return api.types.status.SUCCESS

def Trigger(tc):
    return api.types.status.SUCCESS

def Verify(tc):
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
