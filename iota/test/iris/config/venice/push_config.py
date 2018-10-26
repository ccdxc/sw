#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.cfg_svc_pb2 as cfg_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2

def Main(tc):
    generated_configs = api.GetVeniceConfigs()
    
    req = cfg_svc_pb2.ConfigMsg()
    req.AuthToken = api.GetVeniceAuthToken()
    for cfg_object in generated_configs:
        push_cfg_object = req.configs.add()
        push_cfg_object.method = cfg_object.method
        push_cfg_object.Config = cfg_object.Config

    resp = api.PushConfig(req)
    if resp == None:
        return api.types.status.FAILURE

    return api.types.status.SUCCESS
