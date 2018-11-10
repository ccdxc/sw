#! /usr/bin/python3
import time
import pdb

import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.testcases.storage.pnsodefs as pnsodefs

def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    return api.types.status.SUCCESS

def Verify(tc):
    api.Logger.info("Iterator Info")
    api.Logger.info("- proto: %s" % tc.iterators.proto)
    api.Logger.info("- ipaf : %s" % tc.iterators.ipaf)
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
