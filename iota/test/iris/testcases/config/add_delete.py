#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.topo_svc_pb2 as topo_svc_pb2
import iota.test.iris.config.netagent.cfg_api as netagent_cfg_api


def Setup(tc):
    return api.types.status.SUCCESS


def Trigger(tc):
    netagent_cfg_api.DeleteEndpoints()
    netagent_cfg_api.DeleteNetworks()
    netagent_cfg_api.DeleteSgPolicies()
    netagent_cfg_api.AddNetworks()
    netagent_cfg_api.AddEndpoints()
    netagent_cfg_api.AddSgPolicies()
    return api.types.status.SUCCESS


def Verify(tc):
    result = api.types.status.SUCCESS
    return result


def Teardown(tc):
    return api.types.status.SUCCESS
