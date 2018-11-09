#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api


def Setup(tc):
    return api.types.status.SUCCESS

def Trigger(tc):
    netagent_cfg_api.FlapPorts()
    return api.types.status.SUCCESS

def Verify(tc):
    # TODO Add halctl show port status verification here.
    result = api.types.status.SUCCESS
    return result

def Teardown(tc):
    return api.types.status.SUCCESS
