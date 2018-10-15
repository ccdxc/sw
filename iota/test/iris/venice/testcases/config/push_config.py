#! /usr/bin/python3
import iota.harness.api as api
import iota.protos.pygen.cfg_svc_pb2 as cfg_svc_pb2
import iota.protos.pygen.types_pb2 as types_pb2

def Setup(tc):
    # Prepare a ConfigQueryMsg()
    req = cfg_svc_pb2.ConfigQueryMsg()
    req.view = cfg_svc_pb2.CFG_DB_VIEW_CFG_SVC
    req.kind = "*"
    resp = api.QueryConfig(req)
    if resp == None:    
        return api.types.status.FAILURE

    # Store the config objects in testcase.
    tc.cfg_objects = resp.configs
    return api.types.status.SUCCESS

def Trigger(tc):
    req = cfg_svc_pb2.ConfigMsg()
    for cfg_object in tc.cfg_objects:
        cfg_object.method = cfg_svc_pb2.CFG_METHOD_CREATE
        req.configs.append(cfg_object)

    resp = api.PushConfig(req)
    if resp == None:
        return api.types.status.FAILURE
    return api.types.status.SUCCESS

def Verify(tc):
    # Prepare a ConfigQueryMsg()
    req = cfg_svc_pb2.ConfigQueryMsg()
    req.view = cfg_svc_pb2.CFG_DB_VIEW_TESTBED
    req.kind = "*"
    resp = api.QueryConfig(req)
    if resp == None:
        return api.types.status.FAILURE

    # Validate that testbed view is same as that of config service.
    if resp.configs != tc.cfg_objects:
        api.Logger.error("Testbed config != Service config: ")
        return types.status.FAILURE
    return api.types.status.SUCCESS

def Teardown(tc):
    return api.types.status.SUCCESS
